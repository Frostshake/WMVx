#pragma once

#include <memory>

namespace core {

	class GameFileSystem;
	class DatasetAnimationData;
	class DatasetCharacterRaces;
	class DatasetCharacterFacialHairStyles;
	class DatasetCharacterHairGeosets;
	class DatasetCharacterSections;
	class DatasetCharacterComponentTextures;
	class DatasetCreatureModelData;
	class DatasetCreatureDisplay;
	class DatasetItems;
	class DatasetItemDisplay;
	class DatasetItemVisual;
	class DatasetItemVisualEffect;
	class DatasetSpellItemEnchantment;
	class DatasetItemCache;
	class DatasetNPCs;


	class GameDatabase
	{
	public:
		GameDatabase() = default;
		GameDatabase(GameDatabase&&) = default;
		virtual ~GameDatabase() {}

		virtual void load(const GameFileSystem* const fs) = 0;

		std::unique_ptr<DatasetAnimationData> animationDataDB;
		std::unique_ptr<DatasetCharacterRaces> characterRacesDB;
		std::unique_ptr<DatasetCharacterFacialHairStyles> characterFacialHairStylesDB;
		std::unique_ptr<DatasetCharacterHairGeosets> characterHairGeosetsDB;
		std::unique_ptr<DatasetCharacterSections> characterSectionsDB;
		std::unique_ptr<DatasetCharacterComponentTextures> characterComponentTexturesDB;
		std::unique_ptr<DatasetCreatureModelData> creatureModelDataDB;
		std::unique_ptr<DatasetCreatureDisplay> creatureDisplayDB;
		std::unique_ptr<DatasetItems> itemsDB;
		std::unique_ptr<DatasetItemDisplay> itemDisplayDB;
		std::unique_ptr<DatasetItemVisual> itemVisualsDB;
		std::unique_ptr<DatasetItemVisualEffect> itemVisualEffectsDB;

		std::unique_ptr<DatasetSpellItemEnchantment> spellEnchantmentsDB;

		std::unique_ptr<DatasetNPCs> npcsDB;
	};


};