#pragma once

#include "GameDatabase.h"
#include "FileDataGameDatabase.h"
#include "DFDatasets.h"

namespace core {

	class DFGameDatabase : public GameDatabase, public FileDataGameDatabase<DFDB2ModelFileDataRecord, DFDB2TextureFileDataRecord, DFDB2ComponenetModelFileDataRecord, DFDB2ComponenetTextureFileDataRecord>
	{
	public:
		DFGameDatabase() : GameDatabase() {}
		DFGameDatabase(DFGameDatabase&&) = default;
		virtual ~DFGameDatabase() {}

		void load(const GameFileSystem* const fs) override {

			auto* const cascFS = (CascFileSystem*)(fs);

			loadFileData(cascFS);

			auto items_async = std::async(std::launch::async, [&]() {
				itemsDB = std::make_unique<DFItemDataset>(cascFS);
			});

			auto items_display_async = std::async(std::launch::async, [&]() {
				itemDisplayDB = std::make_unique<DFItemDisplayInfoDataset>(cascFS, this);
			});

			animationDataDB = std::make_unique<DFAnimationDataDataset>(cascFS, "Support Files\\bfa\\animation-names.csv"); //TODO find DF animation names

			auto creatures_async = std::async(std::launch::async, [&]() {
				creatureModelDataDB = std::make_unique<DFCreatureModelDataDataset>(cascFS);
				creatureDisplayDB = std::make_unique<DFCreatureDisplayDataset>(cascFS);
			});

			characterRacesDB = std::make_unique<DFCharRacesDataset>(cascFS);
			characterSectionsDB = nullptr;	//DF has no equivalent

			characterFacialHairStylesDB = std::make_unique<DFCharacterFacialHairStylesDataset>(cascFS);
			characterHairGeosetsDB = std::make_unique<DFCharHairGeosetsDataset>(cascFS);

			characterComponentTexturesDB = std::make_unique<DFCharacterComponentTextureDataset>(cascFS);

			//TODO
			itemVisualsDB = nullptr;
			itemVisualEffectsDB = nullptr;
			spellEnchantmentsDB = nullptr;

			npcsDB = std::make_unique<DFNPCsDataset>(cascFS);

			creatures_async.wait();
			items_async.wait();
			items_display_async.wait();
		}


		
	};
};