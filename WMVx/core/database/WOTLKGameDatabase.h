#pragma once

#include "GameDatabase.h"
#include "WOTLKDatasets.h"
#include "ReferenceSource.h"

namespace core {

	class WOTLKGameDatabase : public GameDatabase
	{
	public:

		WOTLKGameDatabase() : GameDatabase() {}
		WOTLKGameDatabase(WOTLKGameDatabase&&) = default;
		virtual ~WOTLKGameDatabase() {}

		void load(const GameFileSystem* const fs) override {
			auto* const mpqFS = (MPQFileSystem*)(fs);

			animationDataDB = std::make_unique<WOTLKAnimationDataDatasetNext>(mpqFS, "DBFilesClient\\AnimationData.dbc");
			creatureModelDataDB = std::make_unique<WOTLKCreatureModelDataDatasetNext>(mpqFS, "DBFilesClient\\CreatureModelData.dbc");
			creatureDisplayDB = std::make_unique<WOTLKCreatureDisplayDatasetNext>(mpqFS, "DBFilesClient\\CreatureDisplayInfo.dbc", "DBFilesClient\\CreatureDisplayInfoExtra.dbc");
			characterRacesDB = std::make_unique<WOTLKChrRacesDatasetNext>(mpqFS, "DBFilesClient\\ChrRaces.dbc");
			characterSectionsDB = std::make_unique<WOTLKCharSectionsDatasetNext>(mpqFS, "DBFilesClient\\CharSections.dbc");
			characterFacialHairStylesDB = std::make_unique<WOTLKCharacterFacialHairStylesDatasetNext>(mpqFS, "DBFilesClient\\CharacterFacialHairStyles.dbc");
			characterHairGeosetsDB = std::make_unique<WOTLKCharHairGeosetsDatasetNext>(mpqFS, "DBFilesClient\\CharHairGeosets.dbc");

			itemsDB = std::make_unique<WOTLKItemDatasetNext>(mpqFS, "Support Files\\wotlk\\items.csv");
			itemDisplayDB = std::make_unique<WOTLKItemDisplayInfoDatasetNext>(mpqFS, "DBFilesClient\\ItemDisplayInfo.dbc");
			itemVisualsDB = std::make_unique<WOTLKItemVisualDatasetNext>(mpqFS, "DBFilesClient\\ItemVisuals.dbc");
			itemVisualEffectsDB = std::make_unique<WOTLKItemVisualEffectDatasetNext>(mpqFS, "DBFilesClient\\ItemVisualEffects.dbc");
			spellEnchantmentsDB = std::make_unique<WOTLKSpellItemEnchantmentDatasetNext>(mpqFS, "DBFilesClient\\SpellItemEnchantment.dbc");

			characterComponentTexturesDB = nullptr;

			npcsDB = std::make_unique<ReferenceSourceNPCsDataset>("Support Files\\wotlk\\npcs.csv");
		}

	};
};