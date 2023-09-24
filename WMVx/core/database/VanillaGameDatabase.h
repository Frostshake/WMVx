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

			animationDataDB = std::make_unique<VanillaAnimationDataDataset>(mpqFS);
			creatureModelDataDB = std::make_unique<VanillaCreatureModelDataDataset>(mpqFS);
			creatureDisplayDB = std::make_unique<VanillaCreatureDisplayDataset>(mpqFS);
			characterRacesDB = std::make_unique<VanillaCharRacesDataset>(mpqFS);
			characterSectionsDB = std::make_unique<VanillaCharSectionsDataset>(mpqFS);

			characterFacialHairStylesDB = std::make_unique<VanillaCharacterFacialHairStylesDataset>(mpqFS);
			characterHairGeosetsDB = std::make_unique<VanillaCharHairGeosetsDataset>(mpqFS);

			itemsDB = std::make_unique<VanillaItemDataset>(mpqFS, "Support Files\\vanilla\\items.csv");
			itemDisplayDB = std::make_unique<VanillaItemDisplayInfoDataset>(mpqFS);

			itemVisualsDB = std::make_unique<VanillaItemVisualDataset>(mpqFS);
			itemVisualEffectsDB = std::make_unique<VanillaItemVisualEffectDataset>(mpqFS);

			spellEnchantmentsDB = std::make_unique<VanillaSpellItemEnchantmentDataset>(mpqFS);

			characterComponentTexturesDB = nullptr;

			npcsDB = std::make_unique<ReferenceSourceNPCsDataset>("Support Files\\vanilla\\npcs.csv");
		}

		VanillaGameDatabase() : GameDatabase() {}

		VanillaGameDatabase(VanillaGameDatabase&&) = default;
		virtual ~VanillaGameDatabase() {}


	};
};