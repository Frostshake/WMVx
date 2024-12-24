#pragma once

#include "GameDatabase.h"
#include "FileDataGameDatabase.h"
#include "BFADatasets.h"

namespace core {

	class BFAGameDatabase : public GameDatabase, public FileDataGameDatabase<db_bfa::ModelFileDataRecord, db_bfa::TextureFileDataRecord, db_bfa::ComponentModelFileDataRecord, db_bfa::ComponentTextureFileDataRecord>
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

				animationDataDB = std::make_unique<BFAAnimationDataDataset>(cascFS, "dbfilesclient/animationdata.db2", "Support Files\\animation-names.csv");

				auto creatures_async = std::async(std::launch::async, [&]() {
					creatureModelDataDB = std::make_unique<BFACreatureModelDataDataset>(cascFS, "dbfilesclient/creaturemodeldata.db2");
					creatureDisplayDB = std::make_unique<BFACreatureDisplayDataset>(cascFS);
				});

				characterRacesDB = std::make_unique< BFACharRacesDataset>(cascFS, "dbfilesclient/chrraces.db2");
				characterSectionsDB = std::make_unique<BFACharSectionsDataset>(cascFS, this);

				characterFacialHairStylesDB = std::make_unique<BFACharacterFacialHairStylesDataset>(cascFS, "dbfilesclient/characterfacialhairstyles.db2");
				characterHairGeosetsDB = std::make_unique<BFACharHairGeosetsDataset>(cascFS, "dbfilesclient/charhairgeosets.db2");

				characterComponentTexturesDB = std::make_unique<BFACharacterComponentTextureDataset>(cascFS);

				//TODO
				itemVisualsDB = nullptr;
				itemVisualEffectsDB = nullptr;
				spellEnchantmentsDB = nullptr;

				npcsDB = std::make_unique<BFANPCsDataset>(cascFS, "dbfilesclient/creature.db2");

				creatures_async.wait();
				items_async.wait();
				items_display_async.wait();
		}


	};
};