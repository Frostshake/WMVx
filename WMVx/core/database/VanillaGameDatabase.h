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

			animationDataDB = std::make_unique< VanillaAnimationDataDatasetNext>(mpqFS, "DBFilesClient\\AnimationData.dbc");
			creatureModelDataDB = std::make_unique<VanillaCreatureModelDataDatasetNext>(mpqFS, "DBFilesClient\\CreatureModelData.dbc");
			creatureDisplayDB = std::make_unique< VanillaCreatureDisplayDatasetNext>(mpqFS, "DBFilesClient\\CreatureDisplayInfo.dbc", "DBFilesClient\\CreatureDisplayInfoExtra.dbc");
			characterRacesDB = std::make_unique<VanillaChrRacesDatasetNext>(mpqFS, "DBFilesClient\\ChrRaces.dbc");
			characterSectionsDB = std::make_unique<VanillaCharSectionsDatasetNext>(mpqFS, "DBFilesClient\\CharSections.dbc");
			characterFacialHairStylesDB = std::make_unique<VanillaCharacterFacialHairStylesDatasetNext>(mpqFS, "DBFilesClient\\CharacterFacialHairStyles.dbc");
			characterHairGeosetsDB = std::make_unique<VanillaCharHairGeosetsDatasetNext>(mpqFS, "DBFilesClient\\CharHairGeosets.dbc");

			itemsDB = std::make_unique<VanillaItemDatasetNext>(mpqFS, "Support Files\\vanilla\\items.csv");
			itemDisplayDB = std::make_unique<VanillaItemDisplayInfoDatasetNext>(mpqFS, "DBFilesClient\\ItemDisplayInfo.dbc");
			itemVisualsDB = std::make_unique<VanillaItemVisualDatasetNext>(mpqFS, "DBFilesClient\\ItemVisuals.dbc");
			itemVisualEffectsDB = std::make_unique<VanillaItemVisualEffectDatasetNext>(mpqFS, "DBFilesClient\\ItemVisualEffects.dbc");
			spellEnchantmentsDB = std::make_unique<VanillaSpellItemEnchantmentDatasetNext>(mpqFS, "DBFilesClient\\SpellItemEnchantment.dbc");

			characterComponentTexturesDB = nullptr;

			npcsDB = std::make_unique<ReferenceSourceNPCsDataset>("Support Files\\vanilla\\npcs.csv");
		}

		VanillaGameDatabase() = default;
		VanillaGameDatabase(VanillaGameDatabase&&) = default;
		virtual ~VanillaGameDatabase() = default;


	};
};