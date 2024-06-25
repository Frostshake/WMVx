#pragma once

#include "GameDatabase.h"
#include "FileDataGameDatabase.h"
#include "DFDatasets.h"

namespace core {

	class DFGameDatabase : public GameDatabase, public FileDataGameDatabase<db_df::ModelFileDataRecord, db_df::TextureFileDataRecord, db_df::ComponentModelFileDataRecord, db_df::ComponentTextureFileDataRecord>
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

			animationDataDB = std::make_unique<DFAnimationDataDataset>(cascFS, "dbfilesclient/animationdata.db2", "Support Files\\bfa\\animation-names.csv"); //TODO find DF animation names

			auto creatures_async = std::async(std::launch::async, [&]() {
				creatureModelDataDB = std::make_unique<DFCreatureModelDataDataset>(cascFS, "dbfilesclient/creaturemodeldata.db2");
				creatureDisplayDB = std::make_unique<DFCreatureDisplayDataset>(cascFS);
			});

			characterRacesDB = std::make_unique<DFCharRacesDataset>(cascFS, "dbfilesclient/chrraces.db2");
			characterSectionsDB = nullptr;	//DF has no equivalent

			characterFacialHairStylesDB = std::make_unique<DFCharacterFacialHairStylesDataset>(cascFS, "dbfilesclient/characterfacialhairstyles.db2");
			characterHairGeosetsDB = std::make_unique<DFCharHairGeosetsDataset>(cascFS, "dbfilesclient/charhairgeosets.db2");

			characterComponentTexturesDB = std::make_unique<DFCharacterComponentTextureDataset>(cascFS);

			//TODO
			itemVisualsDB = nullptr;
			itemVisualEffectsDB = nullptr;
			spellEnchantmentsDB = nullptr;

			npcsDB = std::make_unique<DFNPCsDataset>(cascFS, "dbfilesclient/creature.db2");

			creatures_async.wait();
			items_async.wait();
			items_display_async.wait();
		}


		
	};
};