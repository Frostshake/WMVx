#pragma once

#include "GameDatabase.h"
#include "VanillaDatasets.h"
#include "ReferenceSource.h"

namespace core {

	class VanillaGameDatabase : public GameDatabase
	{
	public:

		void load(const GameFileSystem* const fs) override {
			auto* const mpqFS = (MPQFileSystem*)(fs);

			animationDataDB = std::make_unique< VanillaAnimationDataDataset>(mpqFS, "DBFilesClient\\AnimationData.dbc");
			creatureModelDataDB = std::make_unique<VanillaCreatureModelDataDataset>(mpqFS, "DBFilesClient\\CreatureModelData.dbc");
			creatureDisplayDB = std::make_unique< VanillaCreatureDisplayDataset>(mpqFS, "DBFilesClient\\CreatureDisplayInfo.dbc", "DBFilesClient\\CreatureDisplayInfoExtra.dbc");
			characterRacesDB = std::make_unique<VanillaChrRacesDataset>(mpqFS, "DBFilesClient\\ChrRaces.dbc");
			characterSectionsDB = std::make_unique<VanillaCharSectionsDataset>(mpqFS, "DBFilesClient\\CharSections.dbc");
			characterFacialHairStylesDB = std::make_unique<VanillaCharacterFacialHairStylesDataset>(mpqFS, "DBFilesClient\\CharacterFacialHairStyles.dbc");
			characterHairGeosetsDB = std::make_unique<VanillaCharHairGeosetsDataset>(mpqFS, "DBFilesClient\\CharHairGeosets.dbc");

			itemsDB = std::make_unique<VanillaItemDataset>(mpqFS, "Support Files\\vanilla\\items.csv");
			itemDisplayDB = std::make_unique<VanillaItemDisplayInfoDataset>(mpqFS, "DBFilesClient\\ItemDisplayInfo.dbc");
			itemVisualsDB = std::make_unique<VanillaItemVisualDataset>(mpqFS, "DBFilesClient\\ItemVisuals.dbc");
			itemVisualEffectsDB = std::make_unique<VanillaItemVisualEffectDataset>(mpqFS, "DBFilesClient\\ItemVisualEffects.dbc");
			spellEnchantmentsDB = std::make_unique<VanillaSpellItemEnchantmentDataset>(mpqFS, "DBFilesClient\\SpellItemEnchantment.dbc");

			characterComponentTexturesDB = nullptr;

			npcsDB = std::make_unique<ReferenceSourceNPCsDataset>("Support Files\\vanilla\\npcs.csv");
		}

		VanillaGameDatabase() = default;
		VanillaGameDatabase(VanillaGameDatabase&&) = default;
		virtual ~VanillaGameDatabase() = default;


	};
};