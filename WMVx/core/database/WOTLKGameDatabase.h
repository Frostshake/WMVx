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

			animationDataDB = std::make_unique<WOTLKAnimationDataDataset>(mpqFS, "DBFilesClient\\AnimationData.dbc");
			creatureModelDataDB = std::make_unique<WOTLKCreatureModelDataDataset>(mpqFS, "DBFilesClient\\CreatureModelData.dbc");
			creatureDisplayDB = std::make_unique<WOTLKCreatureDisplayDataset>(mpqFS, "DBFilesClient\\CreatureDisplayInfo.dbc", "DBFilesClient\\CreatureDisplayInfoExtra.dbc");
			characterRacesDB = std::make_unique<WOTLKChrRacesDataset>(mpqFS, "DBFilesClient\\ChrRaces.dbc");
			characterSectionsDB = std::make_unique<WOTLKCharSectionsDataset>(mpqFS, "DBFilesClient\\CharSections.dbc");
			characterFacialHairStylesDB = std::make_unique<WOTLKCharacterFacialHairStylesDataset>(mpqFS, "DBFilesClient\\CharacterFacialHairStyles.dbc");
			characterHairGeosetsDB = std::make_unique<WOTLKCharHairGeosetsDataset>(mpqFS, "DBFilesClient\\CharHairGeosets.dbc");

			itemsDB = std::make_unique<WOTLKItemDataset>(mpqFS, "Support Files\\wotlk\\items.csv");
			itemDisplayDB = std::make_unique<WOTLKItemDisplayInfoDataset>(mpqFS, "DBFilesClient\\ItemDisplayInfo.dbc");
			itemVisualsDB = std::make_unique<WOTLKItemVisualDataset>(mpqFS, "DBFilesClient\\ItemVisuals.dbc");
			itemVisualEffectsDB = std::make_unique<WOTLKItemVisualEffectDataset>(mpqFS, "DBFilesClient\\ItemVisualEffects.dbc");
			spellEnchantmentsDB = std::make_unique<WOTLKSpellItemEnchantmentDataset>(mpqFS, "DBFilesClient\\SpellItemEnchantment.dbc");

			characterComponentTexturesDB = nullptr;

			npcsDB = std::make_unique<ReferenceSourceNPCsDataset>("Support Files\\wotlk\\npcs.csv");
		}

	};
};