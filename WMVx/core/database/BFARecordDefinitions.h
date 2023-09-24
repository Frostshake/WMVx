#pragma once

#include <cstdint>
#include "../utility/Vector3.h"

#include "DB2Definitions.h"
#include "DB2Schema.h"

namespace core {


#pragma pack(push, 1)

	struct BFADB2AnimationDataRecord {
		struct Data {
			uint32_t id;
			uint16_t fallback;
			uint8_t behaviorTier;
			uint32_t behaviorId;
			uint32_t flags[2];
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.fallback)),
			DB2Field::integer(sizeof(data.behaviorTier)),
			DB2Field::integer(sizeof(data.behaviorId)),
			DB2Field::integerArray(sizeof(data.flags), 2)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2ChrRacesRecord {

		struct Data {
			DB2StringRef clientPrefix;
			DB2StringRef clientFileString;
			DB2LangStringRef nameLang;
			DB2LangStringRef nameFemaleLang;
			DB2LangStringRef nameLowerLang;
			DB2LangStringRef nameFemaleLowerLang;

			uint32_t id;
			uint32_t flags;

			uint32_t maleModelDisplayId;
			uint32_t femaleModelDisplayId;
			uint32_t highResMaleModelDisplayId;
			uint32_t highResFemaleModelDisplayId;

			uint32_t createScreenFileDataId;
			uint32_t selectScreenFileDataId;
			float maleCustomisationOffset[3];
			float femaleCustomisationOffset[3];
			uint32_t lowResScreenFileId;
			uint32_t alteredFormTransitionSpellVisualID[3];
			uint32_t alteredFormTransitionSpellVisualKitID[3];
			int32_t heritageArmorUnlockAchievementID;
			uint32_t startingLevel;
			int32_t uiDisplayOrder;
			int32_t femaleDeathSkeletonModelFileDataID;
			int32_t maleDeathSkeletonModelFileDataID;
			int32_t baseRaceId;
			int32_t transmogDisabledSlotMask;
			uint16_t factionId;
			uint16_t cinematicSequenceId;
			uint16_t resSicknessSpellId;
			uint16_t splashSoundId;
			uint8_t baseLanguage;
			uint8_t creatureType;
			uint8_t alliance;
			uint8_t raceRelated;
			uint8_t unalteredVisualRaceId;
			uint8_t charComponentTextureLayoutID;
			uint8_t charComponentTexLayoutHiResID;
			uint8_t defaultClassID;
			uint8_t neutralRaceID;
			int8_t maleModelFallbackRaceID;
			int8_t maleModelFallbackSex;
			int8_t femaleModelFallbackRaceID;
			int8_t femaleModelFallbackSex;
			int8_t maleTextureFallbackRaceID;
			int8_t maleTextureFallbackSex;
			int8_t femaleTextureFallbackRaceID;
			int8_t femaleTextureFallbackSex;
		} data;

		size_t recordIndex;
	
		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::stringRef(),
			DB2Field::stringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::integer(sizeof(data.id)),
			DB2Field::integer(sizeof(data.flags)),
			DB2Field::integer(sizeof(data.maleModelDisplayId)),
			DB2Field::integer(sizeof(data.femaleModelDisplayId)),
			DB2Field::integer(sizeof(data.highResMaleModelDisplayId)),
			DB2Field::integer(sizeof(data.highResFemaleModelDisplayId)),
			DB2Field::integer(sizeof(data.createScreenFileDataId)),
			DB2Field::integer(sizeof(data.selectScreenFileDataId)),
			DB2Field::floatingPointArray(sizeof(data.maleCustomisationOffset), 3),
			DB2Field::floatingPointArray(sizeof(data.femaleCustomisationOffset), 3),
			DB2Field::integer(sizeof(data.lowResScreenFileId)),
			DB2Field::integerArray(sizeof(data.alteredFormTransitionSpellVisualID), 3),
			DB2Field::integerArray(sizeof(data.alteredFormTransitionSpellVisualKitID), 3),
			DB2Field::integer(sizeof(data.heritageArmorUnlockAchievementID)),
			DB2Field::integer(sizeof(data.startingLevel)),
			DB2Field::integer(sizeof(data.uiDisplayOrder)),
			DB2Field::integer(sizeof(data.femaleDeathSkeletonModelFileDataID)),
			DB2Field::integer(sizeof(data.maleDeathSkeletonModelFileDataID)),
			DB2Field::integer(sizeof(data.baseRaceId)),
			DB2Field::integer(sizeof(data.transmogDisabledSlotMask)),
			DB2Field::integer(sizeof(data.factionId)),
			DB2Field::integer(sizeof(data.cinematicSequenceId)),
			DB2Field::integer(sizeof(data.resSicknessSpellId)),
			DB2Field::integer(sizeof(data.splashSoundId)),
			DB2Field::integer(sizeof(data.baseLanguage)),
			DB2Field::integer(sizeof(data.creatureType)),
			DB2Field::integer(sizeof(data.alliance)),
			DB2Field::integer(sizeof(data.raceRelated)),
			DB2Field::integer(sizeof(data.unalteredVisualRaceId)),
			DB2Field::integer(sizeof(data.charComponentTextureLayoutID)),
			DB2Field::integer(sizeof(data.charComponentTexLayoutHiResID)),
			DB2Field::integer(sizeof(data.defaultClassID)),
			DB2Field::integer(sizeof(data.neutralRaceID)),
			DB2Field::integer(sizeof(data.maleModelFallbackRaceID)),
			DB2Field::integer(sizeof(data.maleModelFallbackSex)),
			DB2Field::integer(sizeof(data.femaleModelFallbackRaceID)),
			DB2Field::integer(sizeof(data.femaleModelFallbackSex)),
			DB2Field::integer(sizeof(data.maleTextureFallbackRaceID)),
			DB2Field::integer(sizeof(data.maleTextureFallbackSex)),
			DB2Field::integer(sizeof(data.femaleTextureFallbackRaceID)),
			DB2Field::integer(sizeof(data.femaleTextureFallbackSex))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2CharSectionsRecord {
		//wow dev wiki and wmv have different names for some of these
		//TODO decide which naming is best.

		struct Data {
			uint32_t id;
			uint8_t raceId;
			uint8_t sexId;
			uint8_t type;
			uint8_t section;
			uint8_t variationIndex;
			uint16_t flags;
			uint32_t textures[3];
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.raceId)),
			DB2Field::integer(sizeof(data.sexId)),
			DB2Field::integer(sizeof(data.type)),
			DB2Field::integer(sizeof(data.variationIndex)),
			DB2Field::integer(sizeof(data.section)),
			DB2Field::integer(sizeof(data.flags)),
			DB2Field::integerArray(sizeof(data.textures), 3)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2CharComponentTextureLayoutsRecord {

		struct Data {
			uint32_t id;
			uint16_t width;
			uint16_t height;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.width)),
			DB2Field::integer(sizeof(data.height))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2CharComponentTextureSectionsRecord {

		struct Data {
			uint32_t id;
			uint8_t charComponentTexturelayoutId;
			uint8_t sectionType;
			uint16_t x;
			uint16_t y;
			uint16_t width;
			uint16_t height;
			uint32_t overlapSectionMask;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.charComponentTexturelayoutId)),
			DB2Field::integer(sizeof(data.sectionType)),
			DB2Field::integer(sizeof(data.x)),
			DB2Field::integer(sizeof(data.y)),
			DB2Field::integer(sizeof(data.width)),
			DB2Field::integer(sizeof(data.height)),
			DB2Field::integer(sizeof(data.overlapSectionMask))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2CharacterFacialHairStylesRecord {

		struct Data {
			uint32_t id;
			uint32_t geosets[5];
			uint8_t raceId;
			uint8_t sexId;
			uint8_t variationId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integerArray(sizeof(data.geosets), 5),
			DB2Field::integer(sizeof(data.raceId)),
			DB2Field::integer(sizeof(data.sexId)),
			DB2Field::integer(sizeof(data.variationId))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2CharHairGeosetsRecord {

		struct Data {
			uint32_t id;
			uint8_t raceId;
			uint8_t sexId;
			uint8_t variationId;
			uint8_t geosetId;
			uint8_t showScalp;
			uint8_t variationType;
			uint8_t geosetType;
			uint8_t colorIndex;
			uint32_t customGeoFileDataId;
			uint32_t HdCustomGeoFileDataId;
			uint8_t unknown;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.raceId)),
			DB2Field::integer(sizeof(data.sexId)),
			DB2Field::integer(sizeof(data.variationId)),
			DB2Field::integer(sizeof(data.geosetId)),
			DB2Field::integer(sizeof(data.showScalp)),
			DB2Field::integer(sizeof(data.variationType)),
			DB2Field::integer(sizeof(data.geosetType)),
			DB2Field::integer(sizeof(data.colorIndex)),
			DB2Field::integer(sizeof(data.customGeoFileDataId)),
			DB2Field::integer(sizeof(data.HdCustomGeoFileDataId)),
			DB2Field::integer(sizeof(data.unknown))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2CreatureModelDataRecord {

		struct Data {
			uint32_t id;
			Vector3 geoBox[2];
			uint32_t flags;
			uint32_t fileDataID;
			uint32_t bloodID;
			uint32_t footprintTextureID;
			float footprintTextureLength;
			float footprintTextureWidth;
			float footprintTextureScale;
			uint32_t foleyMaterialId;
			uint32_t footstepCameraEffectID;
			uint32_t deathThudCameraEffectID;
			uint32_t soundID;
			uint32_t sizeClass;
			float collisionWidth;
			float collisionHeight;
			float worldEffectScale;
			uint32_t creatureGeosetDataID;
			float hoverHeight;
			float attachedEffectScale;
			float modelScale;
			float missileCollisionRadius;
			float missileCollisionPush;
			float missileCollisionRaise;
			float mountHeight;
			float overrideLootEffectScale;
			float overrideNameScale;
			float overrideSelectionRadius;
			float tamedPetBaseScale;
			uint8_t mountScaleOtherIndex;
			float mountScaleSelf;
			float mountScaleOther[2];
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::floatingPointArray(sizeof(data.geoBox), 6),
			DB2Field::integer(sizeof(data.flags)),
			DB2Field::integer(sizeof(data.fileDataID)),
			DB2Field::integer(sizeof(data.bloodID)),
			DB2Field::integer(sizeof(data.footprintTextureID)),
			DB2Field::floatingPoint(sizeof(data.footprintTextureLength)),
			DB2Field::floatingPoint(sizeof(data.footprintTextureWidth)),
			DB2Field::floatingPoint(sizeof(data.footprintTextureScale)),
			DB2Field::integer(sizeof(data.foleyMaterialId)),
			DB2Field::integer(sizeof(data.footstepCameraEffectID)),
			DB2Field::integer(sizeof(data.deathThudCameraEffectID)),
			DB2Field::integer(sizeof(data.soundID)),
			DB2Field::integer(sizeof(data.sizeClass)),
			DB2Field::floatingPoint(sizeof(data.collisionWidth)),
			DB2Field::floatingPoint(sizeof(data.collisionHeight)),
			DB2Field::floatingPoint(sizeof(data.worldEffectScale)),
			DB2Field::integer(sizeof(data.creatureGeosetDataID)),
			DB2Field::floatingPoint(sizeof(data.hoverHeight)),
			DB2Field::floatingPoint(sizeof(data.attachedEffectScale)),
			DB2Field::floatingPoint(sizeof(data.modelScale)),
			DB2Field::floatingPoint(sizeof(data.missileCollisionRadius)),
			DB2Field::floatingPoint(sizeof(data.missileCollisionPush)),
			DB2Field::floatingPoint(sizeof(data.missileCollisionRaise)),
			DB2Field::floatingPoint(sizeof(data.mountHeight)),
			DB2Field::floatingPoint(sizeof(data.overrideLootEffectScale)),
			DB2Field::floatingPoint(sizeof(data.overrideNameScale)),
			DB2Field::floatingPoint(sizeof(data.overrideSelectionRadius)),
			DB2Field::floatingPoint(sizeof(data.tamedPetBaseScale)),
			DB2Field::integer(sizeof(data.mountScaleOtherIndex)),
			DB2Field::floatingPoint(sizeof(data.mountScaleSelf)),
			DB2Field::floatingPointArray(sizeof(data.mountScaleOther), 2)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2CreatureDisplayInfoRecord {

		struct Data {
			uint32_t id;
			uint16_t modelId;
			uint16_t soundId;
			uint8_t sizeClass;
			float scale;
			uint8_t creatureModelAlpha;
			uint8_t bloodId;
			uint32_t extendedDisplayInfoId;
			uint16_t npcSoundId;
			uint16_t particleColorId;
			uint32_t portraitCreatureDisplayInfoId;
			uint32_t portraitTextureFileDataId;
			uint16_t objectEffectPackageId;
			uint16_t animReplacementSetId;
			uint8_t flags;
			uint32_t stateSpellVisualKitId;
			float playerOverrideScale;
			float petInstanceScale;
			uint8_t unarmedWeaponType;
			uint32_t mountPoofSpellVisualKitId;
			uint32_t dissolveEffectId;
			uint8_t gender;
			uint32_t dissolveOutEffectId;
			uint8_t creatureModelMinLod;
			uint32_t textureVariationFileDataID[3];

		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::integer(sizeof(data.id)),
			DB2Field::integer(sizeof(data.modelId)),
			DB2Field::integer(sizeof(data.soundId)),
			DB2Field::integer(sizeof(data.sizeClass)),
			DB2Field::floatingPoint(sizeof(data.scale)),
			DB2Field::integer(sizeof(data.creatureModelAlpha)),
			DB2Field::integer(sizeof(data.bloodId)),
			DB2Field::integer(sizeof(data.extendedDisplayInfoId)),
			DB2Field::integer(sizeof(data.npcSoundId)),
			DB2Field::integer(sizeof(data.particleColorId)),
			DB2Field::integer(sizeof(data.portraitCreatureDisplayInfoId)),
			DB2Field::integer(sizeof(data.portraitTextureFileDataId)),
			DB2Field::integer(sizeof(data.objectEffectPackageId)),
			DB2Field::integer(sizeof(data.animReplacementSetId)),
			DB2Field::integer(sizeof(data.flags)),
			DB2Field::integer(sizeof(data.stateSpellVisualKitId)),
			DB2Field::floatingPoint(sizeof(data.playerOverrideScale)),
			DB2Field::floatingPoint(sizeof(data.petInstanceScale)),
			DB2Field::integer(sizeof(data.unarmedWeaponType)),
			DB2Field::integer(sizeof(data.mountPoofSpellVisualKitId)),
			DB2Field::integer(sizeof(data.dissolveEffectId)),
			DB2Field::integer(sizeof(data.gender)),
			DB2Field::integer(sizeof(data.dissolveOutEffectId)),
			DB2Field::integer(sizeof(data.creatureModelMinLod)),
			DB2Field::integerArray(sizeof(data.textureVariationFileDataID), 3)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");

	};

	struct BFADB2CreatureRecord {

		//TODO check names
		struct Data {
			uint32_t id;
			DB2StringRef nameLang;
			DB2StringRef nameAltLang;
			DB2StringRef titleLang;
			DB2StringRef titleAltLang;
			uint8_t classification;
			uint8_t creatureType;
			uint16_t creatureFamily;
			uint8_t startAnimState;
			uint32_t displayId;
			float displayProbability[4];
			uint32_t alwaysItem[3];
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::stringRef(),
			DB2Field::stringRef(),
			DB2Field::stringRef(),
			DB2Field::stringRef(),
			DB2Field::integer(sizeof(data.classification)),
			DB2Field::integer(sizeof(data.creatureType)),
			DB2Field::integer(sizeof(data.creatureFamily)),
			DB2Field::integer(sizeof(data.startAnimState)),
			DB2Field::integer(sizeof(data.displayId)),
			DB2Field::floatingPointArray(sizeof(data.displayProbability), 4),
			DB2Field::integerArray(sizeof(data.alwaysItem), 3)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2ItemRecord {

		//TODO check names

		struct Data {
			uint32_t id;
			uint8_t classId;
			uint8_t subclassId;
			uint8_t material;
			uint8_t inventoryType;
			uint8_t sheatheType;
			uint8_t soundOverrideSubclassId;
			uint32_t iconFileDataId;
			uint8_t itemGroupSoundsId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.classId)),
			DB2Field::integer(sizeof(data.subclassId)),
			DB2Field::integer(sizeof(data.material)),
			DB2Field::integer(sizeof(data.inventoryType)),
			DB2Field::integer(sizeof(data.sheatheType)),
			DB2Field::integer(sizeof(data.soundOverrideSubclassId)),
			DB2Field::integer(sizeof(data.iconFileDataId)),
			DB2Field::integer(sizeof(data.itemGroupSoundsId))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");

	};

	struct BFADB2ItemDisplayInfoRecord {
		
		//TODO check names
		struct Data {
			uint32_t id;
			uint32_t itemVisualId;
			uint32_t particleColorId;
			uint32_t itemRangedDisplayInfoId;
			uint32_t overrideSwooshSoundKitId;
			uint32_t sheatheTransformMatrixId;
			uint32_t stateSpellVisualKitId;
			uint32_t sheathedSpellVisualKitId;
			uint32_t unsheathedSpellVisualKitId;
			uint32_t flags;
			uint32_t modelResourcesId[2];
			uint32_t modelMaterialResourcesId[2];
			uint8_t modelType[2];
			uint32_t geosetGroup[6];
			uint32_t attachmentGeosetGroup[6];
			uint32_t helmentGeosetVis[2];
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.itemVisualId)),
			DB2Field::integer(sizeof(data.particleColorId)),
			DB2Field::integer(sizeof(data.itemRangedDisplayInfoId)),
			DB2Field::integer(sizeof(data.overrideSwooshSoundKitId)),
			DB2Field::integer(sizeof(data.sheatheTransformMatrixId)),
			DB2Field::integer(sizeof(data.stateSpellVisualKitId)),
			DB2Field::integer(sizeof(data.sheathedSpellVisualKitId)),
			DB2Field::integer(sizeof(data.unsheathedSpellVisualKitId)),
			DB2Field::integer(sizeof(data.flags)),
			DB2Field::integerArray(sizeof(data.modelResourcesId), 2),
			DB2Field::integerArray(sizeof(data.modelMaterialResourcesId), 2),
			DB2Field::integerArray(sizeof(data.modelType), 2),
			DB2Field::integerArray(sizeof(data.geosetGroup), 6),
			DB2Field::integerArray(sizeof(data.attachmentGeosetGroup), 6),
			DB2Field::integerArray(sizeof(data.helmentGeosetVis), 2)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2ItemDisplayInfoMaterialResRecord {
		struct Data {
			uint32_t id;
			uint8_t componentSection;
			uint32_t materialResourcesId;
			uint32_t itemDisplayInfoId;
		 } data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.componentSection)),
			DB2Field::integer(sizeof(data.materialResourcesId)),
			DB2Field::relationship(sizeof(data.itemDisplayInfoId))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2ItemAppearanceRecord {

		struct Data {
			uint32_t id;
			uint8_t displayType;
			uint32_t itemDisplayInfoId;
			uint32_t defaultIconFileDataId;
			uint32_t uiOrder;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.displayType)),
			DB2Field::integer(sizeof(data.itemDisplayInfoId)),
			DB2Field::integer(sizeof(data.defaultIconFileDataId)),
			DB2Field::integer(sizeof(data.uiOrder))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2ItemModifiedAppearanceRecord {

		struct Data {
			uint32_t id;
			uint32_t itemId;
			uint8_t itemAppearanceModifierId;
			uint16_t itemAppearanceId;
			uint8_t orderIndex;
			uint8_t transmogSourceTypeEnum;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::integer(sizeof(data.id)),
			DB2Field::integer(sizeof(data.itemId)),
			DB2Field::integer(sizeof(data.itemAppearanceModifierId)),
			DB2Field::integer(sizeof(data.itemAppearanceId)),
			DB2Field::integer(sizeof(data.orderIndex)),
			DB2Field::integer(sizeof(data.transmogSourceTypeEnum))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2ItemSparseRecord {

		struct Data {
			uint32_t id;
			uint64_t allowableRace;
			//inline strings
			//DB2InlineStringPlaceholder descriptionLang;
			//DB2InlineStringPlaceholder display3Lang;
			//DB2InlineStringPlaceholder display2Lang;
			//DB2InlineStringPlaceholder display1Lang;
			//DB2InlineStringPlaceholder displayLang;
			float dmgVariance;
			uint32_t durationInInventory;
			float qualityModifier;
			uint32_t bagFamily;
			float itemRange;
			float statPercentageOfSocket[10];
			uint32_t statPercentEditor[10];
			uint32_t stackable;
			uint32_t maxCount;
			uint32_t requiredAbility;
			uint32_t sellPrice;
			uint32_t buyPrice;
			uint32_t vendorStackCount;
			float priceVariance;
			float priceRandomValue;
			uint32_t flags[4];
			uint32_t oppositeFactionItemId;
			uint16_t itemNameDescriptionId;
			uint16_t requiredTransmogHoliday;
			uint16_t requiredHoliday;
			uint16_t limitCategory;
			uint16_t gemProperties;
			uint16_t socketMatchEnchantmentId;
			uint16_t totemCategoryId;
			uint16_t instanceBound;
			uint16_t zoneBound[2];
			uint16_t itemSet;
			uint16_t lockId;
			uint16_t startQuestId;
			uint16_t pageId;
			uint16_t itemDelay;
			uint16_t scalingStatDistributionId;
			uint16_t minFactionId;
			uint16_t requiredSkillRank;
			uint16_t requiredSkill;
			uint16_t itemLevel;
			uint16_t allowableClass;
			uint8_t expansionId;
			uint8_t artifactId;
			uint8_t spellWeight;
			uint8_t spellWeightCategory;
			uint8_t socketType[3];
			uint8_t sheatheType;
			uint8_t material;
			uint8_t pageMaterialId;
			uint8_t languageId;
			uint8_t bonding;
			uint8_t damageType;
			uint8_t statModifierBonusStat[10];
			uint8_t containerSlots;
			uint8_t minReputation;
			uint8_t requiredPvpMedal;
			uint8_t requiredPvpRank;
			uint8_t requiredLevel;
			uint8_t inventoryType;
			uint8_t overallQualityId;
		} data;

		size_t recordIndex;	

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.allowableRace)),
			DB2Field::inlineString(),
			DB2Field::inlineString(),
			DB2Field::inlineString(),
			DB2Field::inlineString(),
			DB2Field::inlineString(),
			DB2Field::floatingPoint(sizeof(data.dmgVariance)),
			DB2Field::integer(sizeof(data.durationInInventory)),
			DB2Field::floatingPoint(sizeof(data.qualityModifier)),
			DB2Field::integer(sizeof(data.bagFamily)),
			DB2Field::floatingPoint(sizeof(data.itemRange)),
			DB2Field::floatingPointArray(sizeof(data.statPercentageOfSocket), 10),
			DB2Field::integerArray(sizeof(data.statPercentEditor), 10),
			DB2Field::integer(sizeof(data.stackable)),
			DB2Field::integer(sizeof(data.maxCount)),
			DB2Field::integer(sizeof(data.requiredAbility)),
			DB2Field::integer(sizeof(data.sellPrice)),
			DB2Field::integer(sizeof(data.buyPrice)),
			DB2Field::integer(sizeof(data.vendorStackCount)),
			DB2Field::floatingPoint(sizeof(data.priceVariance)),
			DB2Field::floatingPoint(sizeof(data.priceRandomValue)),
			DB2Field::integerArray(sizeof(data.flags), 4),
			DB2Field::integer(sizeof(data.oppositeFactionItemId)),
			DB2Field::integer(sizeof(data.itemNameDescriptionId)),
			DB2Field::integer(sizeof(data.requiredTransmogHoliday)),
			DB2Field::integer(sizeof(data.requiredHoliday)),
			DB2Field::integer(sizeof(data.limitCategory)),
			DB2Field::integer(sizeof(data.gemProperties)),
			DB2Field::integer(sizeof(data.socketMatchEnchantmentId)),
			DB2Field::integer(sizeof(data.totemCategoryId)),
			DB2Field::integer(sizeof(data.instanceBound)),
			DB2Field::integerArray(sizeof(data.zoneBound), 2),
			DB2Field::integer(sizeof(data.itemSet)),
			DB2Field::integer(sizeof(data.lockId)),
			DB2Field::integer(sizeof(data.startQuestId)),
			DB2Field::integer(sizeof(data.pageId)),
			DB2Field::integer(sizeof(data.itemDelay)),
			DB2Field::integer(sizeof(data.scalingStatDistributionId)),
			DB2Field::integer(sizeof(data.minFactionId)),
			DB2Field::integer(sizeof(data.requiredSkillRank)),
			DB2Field::integer(sizeof(data.requiredSkill)),
			DB2Field::integer(sizeof(data.itemLevel)),
			DB2Field::integer(sizeof(data.allowableClass)),
			DB2Field::integer(sizeof(data.expansionId)),
			DB2Field::integer(sizeof(data.artifactId)),
			DB2Field::integer(sizeof(data.spellWeight)),
			DB2Field::integer(sizeof(data.spellWeightCategory)),
			DB2Field::integerArray(sizeof(data.socketType), 3),
			DB2Field::integer(sizeof(data.sheatheType)),
			DB2Field::integer(sizeof(data.material)),
			DB2Field::integer(sizeof(data.pageMaterialId)),
			DB2Field::integer(sizeof(data.languageId)),
			DB2Field::integer(sizeof(data.bonding)),
			DB2Field::integer(sizeof(data.damageType)),
			DB2Field::integerArray(sizeof(data.statModifierBonusStat), 10),
			DB2Field::integer(sizeof(data.containerSlots)),
			DB2Field::integer(sizeof(data.minReputation)),
			DB2Field::integer(sizeof(data.requiredPvpMedal)),
			DB2Field::integer(sizeof(data.requiredPvpRank)),
			DB2Field::integer(sizeof(data.requiredLevel)),
			DB2Field::integer(sizeof(data.inventoryType)),
			DB2Field::integer(sizeof(data.overallQualityId))
		);

		DB2InlineString inlineStrings[schema.inlineStringCount()];

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2ModelFileDataRecord {
		struct Data {
			uint32_t fileDataId;
			uint8_t flags;
			uint8_t loadCount;
			uint32_t modelResourcesId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::integer(sizeof(data.fileDataId)),
			DB2Field::integer(sizeof(data.flags)),
			DB2Field::integer(sizeof(data.loadCount)),
			DB2Field::integer(sizeof(data.modelResourcesId))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2TextureFileDataRecord {

		struct Data {
			uint32_t fileDataId;
			uint8_t usageType;
			uint32_t materialResourcesId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::integer(sizeof(data.fileDataId)),
			DB2Field::integer(sizeof(data.usageType)),
			DB2Field::integer(sizeof(data.materialResourcesId))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2GuildTabardBackground {

		struct Data {
			uint32_t id;
			uint32_t tier;
			uint32_t component;
			uint32_t fileDataId;
			uint32_t color;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.tier)),
			DB2Field::integer(sizeof(data.component)),
			DB2Field::integer(sizeof(data.fileDataId)),
			DB2Field::relationship(sizeof(data.color))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");

	};

	struct BFADB2GuildTabardBorder {
		struct Data {
			uint32_t id;
			uint32_t borderId;
			uint32_t tier;
			uint32_t component;
			uint32_t fileDataId;
			uint32_t color;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.borderId)),
			DB2Field::integer(sizeof(data.tier)),
			DB2Field::integer(sizeof(data.component)),
			DB2Field::integer(sizeof(data.fileDataId)),
			DB2Field::relationship(sizeof(data.color))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct BFADB2GuildTabardEmblem {
		struct Data {
			uint32_t id;
			uint32_t component;
			uint32_t color;
			uint32_t fileDataId;
			uint32_t emblemId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.component)),
			DB2Field::integer(sizeof(data.color)),
			DB2Field::integer(sizeof(data.fileDataId)),
			DB2Field::relationship(sizeof(data.emblemId))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};



#pragma pack(pop)
};