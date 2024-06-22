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
					itemsDB = std::make_unique<BFAItemDatasetNext>(cascFS);
				});

				auto items_display_async = std::async(std::launch::async, [&]() {
					itemDisplayDB = std::make_unique<BFAItemDisplayInfoDatasetNext>(cascFS, this);
				});

				animationDataDB = std::make_unique<BFAAnimationDataDatasetNext>(cascFS, "dbfilesclient/animationdata.db2", "Support Files\\bfa\\animation-names.csv");

				auto creatures_async = std::async(std::launch::async, [&]() {
					creatureModelDataDB = std::make_unique<BFACreatureModelDataDatasetNext>(cascFS, "dbfilesclient/creaturemodeldata.db2");
					creatureDisplayDB = std::make_unique<BFACreatureDisplayDatasetNext>(cascFS);
				});

				characterRacesDB = std::make_unique< BFACharRacesDatasetNext>(cascFS, "dbfilesclient/chrraces.db2");
				characterSectionsDB = std::make_unique<BFACharSectionsDatasetNext>(cascFS, this);

				characterFacialHairStylesDB = std::make_unique<BFACharacterFacialHairStylesDatasetNext>(cascFS, "dbfilesclient/characterfacialhairstyles.db2");
				characterHairGeosetsDB = std::make_unique<BFACharHairGeosetsDatasetNext>(cascFS, "dbfilesclient/charhairgeosets.db2");

				characterComponentTexturesDB = std::make_unique<BFACharacterComponentTextureDatasetNext>(cascFS);

				//TODO
				itemVisualsDB = nullptr;
				itemVisualEffectsDB = nullptr;
				spellEnchantmentsDB = nullptr;

				npcsDB = std::make_unique<BFANPCsDatasetNext>(cascFS, "dbfilesclient/creature.db2");

				creatures_async.wait();
				items_async.wait();
				items_display_async.wait();
		}


	};
};