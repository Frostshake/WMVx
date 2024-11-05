#pragma once

#include "GameDatabase.h"
#include "CataDatasets.h"
#include "ReferenceSource.h"

namespace core {

	class CataGameDatabase : public GameDatabase
	{
	public:

		CataGameDatabase() : GameDatabase() {}
		CataGameDatabase(CataGameDatabase&&) = default;
		virtual ~CataGameDatabase() {}

		void load(const GameFileSystem* const fs) override {
			auto* const mpqFS = (MPQFileSystem*)(fs);

			animationDataDB = std::make_unique<CataAnimationDataDataset>(mpqFS, "DBFilesClient\\AnimationData.dbc");
			creatureModelDataDB = std::make_unique<CataCreatureModelDataDataset>(mpqFS, "DBFilesClient\\CreatureModelData.dbc");
			creatureDisplayDB = std::make_unique<CataCreatureDisplayDataset>(mpqFS, "DBFilesClient\\CreatureDisplayInfo.dbc", "DBFilesClient\\CreatureDisplayInfoExtra.dbc");
			characterRacesDB = std::make_unique<CataChrRacesDataset>(mpqFS, "DBFilesClient\\ChrRaces.dbc");
			characterSectionsDB = std::make_unique<CataCharSectionsDataset>(mpqFS, "DBFilesClient\\CharSections.dbc");
			characterFacialHairStylesDB = std::make_unique<CataCharacterFacialHairStylesDataset>(mpqFS, "DBFilesClient\\CharacterFacialHairStyles.dbc");
			characterHairGeosetsDB = std::make_unique<CataCharHairGeosetsDataset>(mpqFS, "DBFilesClient\\CharHairGeosets.dbc");

			itemsDB = std::make_unique<CataItemDataset>(mpqFS, "Support Files\\wotlk\\items.csv");
			itemDisplayDB = std::make_unique<CataItemDisplayInfoDataset>(mpqFS, "DBFilesClient\\ItemDisplayInfo.dbc");
			itemVisualsDB = std::make_unique<CataItemVisualDataset>(mpqFS, "DBFilesClient\\ItemVisuals.dbc");
			itemVisualEffectsDB = std::make_unique<CataItemVisualEffectDataset>(mpqFS, "DBFilesClient\\ItemVisualEffects.dbc");
			spellEnchantmentsDB = std::make_unique<CataSpellItemEnchantmentDataset>(mpqFS, "DBFilesClient\\SpellItemEnchantment.dbc");

			characterComponentTexturesDB = nullptr;

			npcsDB = std::make_unique<ReferenceSourceNPCsDataset>("Support Files\\wotlk\\npcs.csv");	//TODO Cata file.
		}

	};
};