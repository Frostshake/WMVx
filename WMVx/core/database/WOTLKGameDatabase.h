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

			animationDataDB = std::make_unique<WOTLKAnimationDataDataset>(mpqFS);
			creatureModelDataDB = std::make_unique<WOTLKCreatureModelDataDataset>(mpqFS);
			creatureDisplayDB = std::make_unique<WOTLKCreatureDisplayDataset>(mpqFS);
			characterRacesDB = std::make_unique<WOTLKCharRacesDataset>(mpqFS);
			characterSectionsDB = std::make_unique<WOTLKCharSectionsDataset>(mpqFS);

			characterFacialHairStylesDB = std::make_unique<WOTLKCharacterFacialHairStylesDataset>(mpqFS);
			characterHairGeosetsDB = std::make_unique<WOTLKCharHairGeosetsDataset>(mpqFS);

			itemsDB = std::make_unique<WOTLKItemDataset>(mpqFS, "Support Files\\wotlk\\items.csv");
			itemDisplayDB = std::make_unique<WOTLKItemDisplayInfoDataset>(mpqFS);

			itemVisualsDB = std::make_unique<WOTLKItemVisualDataset>(mpqFS);
			itemVisualEffectsDB = std::make_unique<WOTLKItemVisualEffectDataset>(mpqFS);

			spellEnchantmentsDB = std::make_unique<WOTLKSpellItemEnchantmentDataset>(mpqFS);

			characterComponentTexturesDB = nullptr;

			npcsDB = std::make_unique<ReferenceSourceNPCsDataset>("Support Files\\wotlk\\npcs.csv");
		}

	};
};