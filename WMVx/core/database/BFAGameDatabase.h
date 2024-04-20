#pragma once

#include "GameDatabase.h"
#include "FileDataGameDatabase.h"
#include "BFADatasets.h"

namespace core {

	class BFAGameDatabase : public GameDatabase, public FileDataGameDatabase<BFADB2ModelFileDataRecord, BFADB2TextureFileDataRecord, BFADB2ComponenetModelFileDataRecord, BFADB2ComponenetTextureFileDataRecord>
	{
	public:
		BFAGameDatabase() : GameDatabase() {}
		BFAGameDatabase(BFAGameDatabase&&) = default;
		virtual ~BFAGameDatabase() {}

		void load(const GameFileSystem* const fs) override {

				auto* const cascFS = (CascFileSystem*)(fs);

				loadFileData(cascFS);

				auto items_async = std::async(std::launch::async, [&]() {
					itemsDB = std::make_unique<BFAItemDataset>(cascFS);
				});

				auto items_display_async = std::async(std::launch::async, [&]() {
					itemDisplayDB = std::make_unique<BFAItemDisplayInfoDataset>(cascFS, this);
				});

				animationDataDB = std::make_unique<BFAAnimationDataDataset>(cascFS, "Support Files\\bfa\\animation-names.csv");

				auto creatures_async = std::async(std::launch::async, [&]() {
					creatureModelDataDB = std::make_unique<BFACreatureModelDataDataset>(cascFS);
					creatureDisplayDB = std::make_unique<BFACreatureDisplayDataset>(cascFS);
				});

				characterRacesDB = std::make_unique<BFACharRacesDataset>(cascFS);
				characterSectionsDB = std::make_unique<BFACharSectionsDataset>(cascFS, this);

				characterFacialHairStylesDB = std::make_unique<BFACharacterFacialHairStylesDataset>(cascFS);
				characterHairGeosetsDB = std::make_unique<BFACharHairGeosetsDataset>(cascFS);

				characterComponentTexturesDB = std::make_unique<BFACharacterComponentTextureDataset>(cascFS);

				//itemVisualsDB
				//itemVisualEffectsDB
				//spellEnchantmentsDB
				//TODO

				spellEnchantmentsDB = nullptr;

				npcsDB = std::make_unique<BFANPCsDataset>(cascFS);

				creatures_async.wait();
				items_async.wait();
				items_display_async.wait();
		}


	};
};