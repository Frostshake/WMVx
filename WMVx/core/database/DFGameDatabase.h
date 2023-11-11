#pragma once

#include "GameDatabase.h"
#include "DFDatasets.h"

namespace core {

	class DFGameDatabase : public GameDatabase
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
			characterSectionsDB = std::make_unique<DFCharSectionsDataset>(cascFS, textureFileDataDB.get());

			//TODO facial hair
			//TODO hair

			characterComponentTexturesDB = std::make_unique<DFCharacterComponentTextureDataset>(cascFS);

			// ...

			creatures_async.wait();

		}

	protected:

		std::unique_ptr<DB2File<DFDB2TextureFileDataRecord>> textureFileDataDB;


	};
};