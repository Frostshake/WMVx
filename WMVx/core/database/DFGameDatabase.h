#pragma once

#include "GameDatabase.h"
#include "FileDataGameDatabase.h"
#include "DFDatasets.h"

namespace core {

	class DFGameDatabase : public GameDatabase, public FileDataGameDatabase
	{
	public:
		DFGameDatabase() : GameDatabase() {}
		DFGameDatabase(DFGameDatabase&&) = default;
		virtual ~DFGameDatabase() {}

		void load(const GameFileSystem* const fs) override {

			auto* const cascFS = (CascFileSystem*)(fs);

			textureFileDataDB = std::make_unique<DB2File<DFDB2TextureFileDataRecord>>("dbfilesclient/texturefiledata.db2");
			textureFileDataDB->open(cascFS);

			animationDataDB = std::make_unique<DFAnimationDataDataset>(cascFS, "Support Files\\bfa\\animation-names.csv"); //TODO find DF animation names

			auto creatures_async = std::async(std::launch::async, [&]() {
				creatureModelDataDB = std::make_unique<DFCreatureModelDataDataset>(cascFS);
				creatureDisplayDB = std::make_unique<DFCreatureDisplayDataset>(cascFS);
			});

			characterRacesDB = std::make_unique<DFCharRacesDataset>(cascFS);
			characterSectionsDB = std::make_unique<DFCharSectionsDataset>(cascFS, this);

			characterFacialHairStylesDB = std::make_unique<DFCharacterFacialHairStylesDataset>(cascFS);
			characterHairGeosetsDB = std::make_unique<DFCharHairGeosetsDataset>(cascFS);

			characterComponentTexturesDB = std::make_unique<DFCharacterComponentTextureDataset>(cascFS);


			creatures_async.wait();

		}


		virtual GameFileUri::id_t findByMaterialResId(uint32_t id) const {
			const auto& sections = textureFileDataDB->getSections();
			for (const auto& section : sections) {
				for (const auto& record : section.records) {
					if (record.data.materialResourcesId == id) {
						return record.data.fileDataId;
					}
				}
			}

			return 0u;
		}

		virtual GameFileUri::id_t findByModelResId(uint32_t id) const {
			//TODO
			return 0u;
		}


	protected:		
		std::unique_ptr<DB2File<DFDB2TextureFileDataRecord>> textureFileDataDB;
		
	};
};