#pragma once

#include "GameDatabase.h"
#include "TBCDatasets.h"
#include "ReferenceSource.h"

namespace core {

	class TBCGameDatabase : public GameDatabase
	{
	public:

		TBCGameDatabase() : GameDatabase() {}
		TBCGameDatabase(TBCGameDatabase&&) = default;
		virtual ~TBCGameDatabase() {}

		void load(const GameFileSystem* const fs) override {
			auto* const mpqFS = (MPQFileSystem*)(fs);

			animationDataDB = std::make_unique<TBCAnimationDataDataset>(mpqFS, "DBFilesClient\\AnimationData.dbc");
			creatureModelDataDB = std::make_unique<TBCCreatureModelDataDataset>(mpqFS, "DBFilesClient\\CreatureModelData.dbc");
			creatureDisplayDB = std::make_unique<TBCCreatureDisplayDataset>(mpqFS, "DBFilesClient\\CreatureDisplayInfo.dbc", "DBFilesClient\\CreatureDisplayInfoExtra.dbc");
			characterRacesDB = std::make_unique<TBCChrRacesDataset>(mpqFS, "DBFilesClient\\ChrRaces.dbc");
			characterSectionsDB = std::make_unique<TBCCharSectionsDataset>(mpqFS, "DBFilesClient\\CharSections.dbc");
			characterFacialHairStylesDB = std::make_unique<TBCCharacterFacialHairStylesDataset>(mpqFS, "DBFilesClient\\CharacterFacialHairStyles.dbc");
			characterHairGeosetsDB = std::make_unique<TBCCharHairGeosetsDataset>(mpqFS, "DBFilesClient\\CharHairGeosets.dbc");

			itemsDB = std::make_unique<TBCItemDataset>(mpqFS, "Support Files\\wotlk\\items.csv");
			itemDisplayDB = std::make_unique<TBCItemDisplayInfoDataset>(mpqFS, "DBFilesClient\\ItemDisplayInfo.dbc");
			itemVisualsDB = std::make_unique<TBCItemVisualDataset>(mpqFS, "DBFilesClient\\ItemVisuals.dbc");
			itemVisualEffectsDB = std::make_unique<TBCItemVisualEffectDataset>(mpqFS, "DBFilesClient\\ItemVisualEffects.dbc");
			spellEnchantmentsDB = std::make_unique<TBCSpellItemEnchantmentDataset>(mpqFS, "DBFilesClient\\SpellItemEnchantment.dbc");

			characterComponentTexturesDB = nullptr;

			npcsDB = std::make_unique<ReferenceSourceNPCsDataset>("Support Files\\wotlk\\npcs.csv");	//TODO TBC file.
		}

	};
};