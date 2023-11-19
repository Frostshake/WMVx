#pragma once

#include <cstdint>
#include "../utility/Vector3.h"

#include "DB2Definitions.h"
#include "DB2Schema.h"

#include "BFARecordDefinitions.h"

namespace core {

#pragma pack(push, 1)

	using DFDB2AnimationDataRecord = BFADB2AnimationDataRecord;

	struct DFDB2ChrRacesRecord {

		struct Data {
			uint32_t id;
			DB2StringRef clientPrefix;
			DB2StringRef clientFileString;

			DB2LangStringRef nameLang;
			DB2LangStringRef nameFemaleLang;
			DB2LangStringRef nameLowerLang;
			DB2LangStringRef nameFemaleLowerLang;

			DB2LangStringRef loreNameLang;
			DB2LangStringRef loreNameFemaleLang;
			DB2LangStringRef loreNameLowerLang;
			DB2LangStringRef loreNameFemaleLowerLang;

			DB2LangStringRef loreDescriptionLang;

			DB2LangStringRef shortNameLang;
			DB2LangStringRef shortNameFemaleLang;
			DB2LangStringRef shortNameLowerLang;
			DB2LangStringRef shortNameFemaleLowerLang;

			
			uint32_t flags;
			uint32_t factionId;
			uint32_t cinematicSequenceId;
			uint32_t resSicknessSpellId;
			uint32_t splashSoundId;
			uint32_t alliance;
			uint32_t raceRelated;
			uint32_t unalteredVisualRaceId;
			uint32_t defaultClassID;
			uint32_t createScreenFileDataId;
			uint32_t selectScreenFileDataId;
			uint32_t neutralRaceID;
			uint32_t lowResScreenFileId;
			uint32_t alteredFormTransitionSpellVisualID[3];
			uint32_t alteredFormTransitionSpellVisualKitID[3];

			int32_t heritageArmorUnlockAchievementID;
			uint32_t startingLevel;
			int32_t uiDisplayOrder;
			int32_t maleModelFallbackRaceID;
			int32_t femaleModelFallbackRaceID;
			int32_t maleTextureFallbackRaceID;
			int32_t femaleTextureFallbackRaceID;
			uint32_t playableRaceBit;
			uint32_t helmetAnimScalingRaceID;
			int32_t transmogDisabledSlotMask;

			uint32_t unalteredVisualCustomizationRaceID;
			uint32_t alteredFormCustomizeOffsetFallback[3];
			uint32_t alteredFormCustomizeRotationFallback;
			float unknown_9_1_0_38312_030[3];
			float unknown_9_1_0_38312_031[3];
			uint32_t unknown_10_0_0_44649_045;
			
			uint8_t baseLanguage;
			uint8_t creatureType;
			int8_t maleModelFallbackSex;
			int8_t femaleModelFallbackSex;
			int8_t maleTextureFallbackSex;
			int8_t femaleTextureFallbackSex;			

		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::stringRef(),
			DB2Field::stringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::langStringRef(),
			DB2Field::integer(sizeof(data.flags)),
			DB2Field::integer(sizeof(data.factionId)),
			DB2Field::integer(sizeof(data.cinematicSequenceId)),
			DB2Field::integer(sizeof(data.resSicknessSpellId)),
			DB2Field::integer(sizeof(data.splashSoundId)),
			DB2Field::integer(sizeof(data.alliance)),
			DB2Field::integer(sizeof(data.raceRelated)),
			DB2Field::integer(sizeof(data.unalteredVisualRaceId)),
			DB2Field::integer(sizeof(data.defaultClassID)),
			DB2Field::integer(sizeof(data.createScreenFileDataId)),
			DB2Field::integer(sizeof(data.selectScreenFileDataId)),
			DB2Field::integer(sizeof(data.neutralRaceID)),
			DB2Field::integer(sizeof(data.lowResScreenFileId)),
			DB2Field::integerArray(sizeof(data.alteredFormTransitionSpellVisualID), 3),
			DB2Field::integerArray(sizeof(data.alteredFormTransitionSpellVisualKitID), 3),
			DB2Field::integer(sizeof(data.heritageArmorUnlockAchievementID)),
			DB2Field::integer(sizeof(data.startingLevel)),
			DB2Field::integer(sizeof(data.uiDisplayOrder)),
			DB2Field::integer(sizeof(data.maleModelFallbackRaceID)),
			DB2Field::integer(sizeof(data.femaleModelFallbackRaceID)),
			DB2Field::integer(sizeof(data.maleTextureFallbackRaceID)),
			DB2Field::integer(sizeof(data.femaleTextureFallbackRaceID)),
			DB2Field::integer(sizeof(data.playableRaceBit)),
			DB2Field::integer(sizeof(data.helmetAnimScalingRaceID)),
			DB2Field::integer(sizeof(data.transmogDisabledSlotMask)),
			DB2Field::integer(sizeof(data.unalteredVisualCustomizationRaceID)),
			DB2Field::integerArray(sizeof(data.alteredFormCustomizeOffsetFallback), 3),
			DB2Field::integer(sizeof(data.alteredFormCustomizeRotationFallback)),
			DB2Field::floatingPointArray(sizeof(data.unknown_9_1_0_38312_030), 3),
			DB2Field::floatingPointArray(sizeof(data.unknown_9_1_0_38312_031), 3),
			DB2Field::integer(sizeof(data.unknown_10_0_0_44649_045)),
			DB2Field::integer(sizeof(data.baseLanguage)),
			DB2Field::integer(sizeof(data.creatureType)),
			DB2Field::integer(sizeof(data.maleModelFallbackSex)),
			DB2Field::integer(sizeof(data.femaleModelFallbackSex)),
			DB2Field::integer(sizeof(data.maleTextureFallbackSex)),
			DB2Field::integer(sizeof(data.femaleTextureFallbackSex))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ChrModelRecord {
		struct Data {
			float faceCustomizationOffset[3];
			float customizeOffset[3];
			uint32_t id;
			uint8_t sex;
			uint32_t displayID;
			uint32_t charComponentTextureLayoutID;
			uint32_t flags;
			uint32_t skeletonFileDataID;
			uint32_t modelFallbackChrModelID;
			uint32_t textureFallbackChrModelID;
			uint32_t helmVisFallbackChrModelID;
			float customizeScale;
			float customizeFacing;
			float cameraDistanceOffset;
			float barberShopCameraOffsetScale;
			float barberShopCameraRotationFacing;
			float barberShopCameraRotationOffset;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldValue(Data::faceCustomizationOffset),
			DB2FieldValue(Data::customizeOffset),
			DB2FieldValue(Data::id),	// TODO inline id
			DB2FieldValue(Data::sex),
			DB2FieldValue(Data::displayID), //TODO inline relation
			DB2FieldValue(Data::charComponentTextureLayoutID),
			DB2FieldValue(Data::flags),
			DB2FieldValue(Data::skeletonFileDataID),
			DB2FieldValue(Data::modelFallbackChrModelID),
			DB2FieldValue(Data::textureFallbackChrModelID),
			DB2FieldValue(Data::helmVisFallbackChrModelID),
			DB2FieldValue(Data::customizeScale),
			DB2FieldValue(Data::customizeFacing),
			DB2FieldValue(Data::cameraDistanceOffset),
			DB2FieldValue(Data::barberShopCameraOffsetScale),
			DB2FieldValue(Data::barberShopCameraRotationFacing),
			DB2FieldValue(Data::barberShopCameraRotationOffset)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ChrCustomizationRecord {
		struct Data {
			uint32_t id;
			DB2LangStringRef nameLang;
			uint32_t sex;
			int32_t baseSection;
			int32_t uiCustomizationType;
			uint32_t flags;
			int32_t componentSection[3];
			uint32_t raceId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldLangStringRef(Data::nameLang),
			DB2FieldValue(Data::sex),
			DB2FieldValue(Data::baseSection),
			DB2FieldValue(Data::uiCustomizationType),
			DB2FieldValue(Data::flags),
			DB2FieldValue(Data::componentSection),
			DB2FieldRelation(Data::raceId)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ChrCustomizationOptionRecord {
		struct Data {
			DB2LangStringRef nameLang;
			uint32_t id;
			uint16_t secondaryId;
			uint32_t flags;
			uint32_t chrModelId;
			uint32_t orderIndex;
			uint32_t chrCustomizationCategoryId;
			uint32_t optionType;
			float barberShopCostModifier;
			uint32_t chrCustomizationId;
			uint32_t requirement;
			uint32_t secondaryOrderIndex;
			uint32_t addedInPatch;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldLangStringRef(Data::nameLang),
			DB2FieldValue(Data::id),	//TODO inline id
			DB2FieldValue(Data::secondaryId),
			DB2FieldValue(Data::flags),
			DB2FieldValue(Data::chrModelId), //TODO inline relation
			DB2FieldValue(Data::orderIndex),
			DB2FieldValue(Data::chrCustomizationCategoryId),
			DB2FieldValue(Data::optionType),
			DB2FieldValue(Data::barberShopCostModifier),
			DB2FieldValue(Data::chrCustomizationId),
			DB2FieldValue(Data::requirement),
			DB2FieldValue(Data::secondaryOrderIndex),
			DB2FieldValue(Data::addedInPatch)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ChrCustomizationChoiceRecord {
		struct Data {
			DB2LangStringRef nameLang;
			uint32_t id;
			uint32_t chrCustomizationOptionId;
			uint32_t chrCustomizationReqId;
			uint32_t chrCustomizationVisReqId;
			uint16_t orderIndex;
			uint16_t uiOrderIndex;
			uint32_t flags;
			uint32_t addedInPatch;
			uint32_t soundKitId;
			uint32_t swatchColor[2];
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldLangStringRef(Data::nameLang),
			DB2FieldValue(Data::id),	//TODO inline id
			DB2FieldValue(Data::chrCustomizationOptionId), //TODO inline relation
			DB2FieldValue(Data::chrCustomizationReqId),
			DB2FieldValue(Data::chrCustomizationVisReqId),
			DB2FieldValue(Data::orderIndex),
			DB2FieldValue(Data::uiOrderIndex),
			DB2FieldValue(Data::flags),
			DB2FieldValue(Data::addedInPatch),
			DB2FieldValue(Data::soundKitId),
			DB2FieldValue(Data::swatchColor)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ChrCustomizationElementRecord {
		struct Data {
			uint32_t id;
			uint32_t chrCustomizationChoiceId;
			uint32_t relatedChrCustomizationChoiceId;
			uint32_t chrCustomizationGeosetId;
			uint32_t chrCustomizationSkinnedModelId;
			uint32_t chrCustomizationMaterialId;
			uint32_t chrCustomizationBoneSetId;
			uint32_t chrCustomizationCondModelId;
			uint32_t chrCustomizationDisplayInfoId;
			uint32_t chrCustItemGeoModifyId;
			uint32_t chrCustomizationVoiceId;
			uint32_t animKitId;
			uint32_t particleColorId;
			uint32_t chrCustGeoComponentLinkId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldValue(Data::chrCustomizationChoiceId),
			DB2FieldValue(Data::relatedChrCustomizationChoiceId),
			DB2FieldValue(Data::chrCustomizationGeosetId),
			DB2FieldValue(Data::chrCustomizationSkinnedModelId),
			DB2FieldValue(Data::chrCustomizationMaterialId),
			DB2FieldValue(Data::chrCustomizationBoneSetId),
			DB2FieldValue(Data::chrCustomizationCondModelId),
			DB2FieldValue(Data::chrCustomizationDisplayInfoId),
			DB2FieldValue(Data::chrCustItemGeoModifyId),
			DB2FieldValue(Data::chrCustomizationVoiceId),
			DB2FieldValue(Data::animKitId),
			DB2FieldValue(Data::particleColorId),
			DB2FieldValue(Data::chrCustGeoComponentLinkId)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};


	struct DFDB2ChrCustomizationGeosetRecord {
		struct Data {
			uint32_t id;
			int32_t geosetType;
			int32_t geosetId;
			int32_t modifier;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldValue(Data::geosetType),
			DB2FieldValue(Data::geosetId),
			DB2FieldValue(Data::modifier)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ChrCustomizationSkinnedModelRecord {
		struct Data {
			uint32_t id;
			uint32_t collectionsFileDataId;
			int32_t geosetType;
			int32_t geosetId;
			int32_t modifier;
			uint32_t flags;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldValue(Data::collectionsFileDataId),
			DB2FieldValue(Data::geosetType),
			DB2FieldValue(Data::geosetId),
			DB2FieldValue(Data::modifier),
			DB2FieldValue(Data::flags)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ChrCustomizationMaterialRecord {
		struct Data {
			uint32_t id;
			uint32_t chrModelTextureTargetId;
			uint32_t materialResourcesId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldValue(Data::chrModelTextureTargetId),
			DB2FieldValue(Data::materialResourcesId)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ChrRaceXChrModelRecord {
		struct Data {
			uint32_t id;
			uint32_t raceId;
			uint32_t chrModelId;
			uint32_t sex;
			uint32_t allowedTransmogSlots;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldValue(Data::raceId), //TODO inline relation
			DB2FieldValue(Data::chrModelId),
			DB2FieldValue(Data::sex),
			DB2FieldValue(Data::allowedTransmogSlots)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ChrModelTextureLayerRecord {
		struct Data {
			uint32_t id;
			int32_t textureType;
			int32_t layer;
			uint32_t flags;
			int32_t blendMode;
			uint32_t textureSectionTypeBitMask;
			uint32_t textureSectionTypeBitMask2;
			uint32_t unknown_9_0_1[3];
			uint32_t chrModelTextureTargetId[2];
			uint32_t chrComponentTextureLayoutId;

		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldValue(Data::textureType),
			DB2FieldValue(Data::layer),
			DB2FieldValue(Data::flags),
			DB2FieldValue(Data::blendMode),
			DB2FieldValue(Data::textureSectionTypeBitMask),
			DB2FieldValue(Data::textureSectionTypeBitMask2),
			DB2FieldValue(Data::unknown_9_0_1),
			DB2FieldValue(Data::chrModelTextureTargetId),
			DB2FieldRelation(Data::chrComponentTextureLayoutId)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};


	using DFDB2CharBaseSectionRecord = BFADB2CharBaseSectionRecord;

	struct DFDB2CharSectionConditionRecord {
		//wow dev wiki and wmv have different names for some of these
		//TODO decide which naming is best.

		struct Data {
			uint32_t id;
			uint8_t baseSectionId;
			uint8_t sexId;
			uint8_t variationIndex;
			uint8_t section;
			uint32_t acheivementID;
			uint32_t raceId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.baseSectionId)),
			DB2Field::integer(sizeof(data.sexId)),
			DB2Field::integer(sizeof(data.variationIndex)),
			DB2Field::integer(sizeof(data.section)),
			DB2Field::integer(sizeof(data.acheivementID)),
			DB2Field::relationship(sizeof(data.raceId))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2CharComponentTextureLayoutsRecord {

		struct Data {
			uint32_t id;
			uint32_t width;
			uint32_t height;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2Field::id(sizeof(data.id)),
			DB2Field::integer(sizeof(data.width)),
			DB2Field::integer(sizeof(data.height))
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2CharComponentTextureSectionsRecord {

		struct Data {
			uint32_t id;
			uint32_t charComponentTexturelayoutId;
			uint32_t sectionType;
			uint32_t x;
			uint32_t y;
			uint32_t width;
			uint32_t height;
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

	using DFDB2CharacterFacialHairStylesRecord = BFADB2CharacterFacialHairStylesRecord;

	using DFDB2CharHairGeosetsRecord = BFADB2CharHairGeosetsRecord;

	struct DFDB2CreatureModelDataRecord {

		struct Data {
			uint32_t id;
			Vector3 geoBox[2];
			uint32_t flags;
			uint32_t fileDataID;
			float walkSpeed;
			float runSpeed;
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
			DB2Field::floatingPoint(sizeof(data.walkSpeed)),
			DB2Field::floatingPoint(sizeof(data.runSpeed)),
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

	struct DFDB2CreatureDisplayInfoRecord {

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
			uint32_t textureVariationFileDataID[4];

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
			DB2Field::integerArray(sizeof(data.textureVariationFileDataID), 4)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");

	};

	struct DFDB2CreatureRecord {

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
			uint32_t displayId[4];
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
			DB2Field::integerArray(sizeof(data.displayId), 4),
			DB2Field::floatingPointArray(sizeof(data.displayProbability), 4),
			DB2Field::integerArray(sizeof(data.alwaysItem), 3)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ItemRecord {
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
			int32_t contentTuningID;
			int32_t modifiedCraftingReagentItemId;
			int32_t craftingQualityId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldValue(Data::classId),
			DB2FieldValue(Data::subclassId),
			DB2FieldValue(Data::material),
			DB2FieldValue(Data::inventoryType),
			DB2FieldValue(Data::sheatheType),
			DB2FieldValue(Data::soundOverrideSubclassId),
			DB2FieldValue(Data::iconFileDataId),
			DB2FieldValue(Data::itemGroupSoundsId),
			DB2FieldValue(Data::contentTuningID),
			DB2FieldValue(Data::modifiedCraftingReagentItemId),
			DB2FieldValue(Data::craftingQualityId)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ItemDisplayInfoRecord {
		struct Data {
			uint32_t id;
			int32_t geosetGroupOverride;
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
			uint32_t modelType[2];
			uint32_t geosetGroup[6];
			uint32_t attachmentGeosetGroup[6];
			uint32_t helmentGeosetVis[2];
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldValue(Data::geosetGroupOverride),
			DB2FieldValue(Data::itemVisualId),
			DB2FieldValue(Data::particleColorId),
			DB2FieldValue(Data::itemRangedDisplayInfoId),
			DB2FieldValue(Data::overrideSwooshSoundKitId),
			DB2FieldValue(Data::sheatheTransformMatrixId),
			DB2FieldValue(Data::stateSpellVisualKitId),
			DB2FieldValue(Data::sheathedSpellVisualKitId),
			DB2FieldValue(Data::unsheathedSpellVisualKitId),
			DB2FieldValue(Data::flags),
			DB2FieldValue(Data::modelResourcesId),
			DB2FieldValue(Data::modelMaterialResourcesId),
			DB2FieldValue(Data::modelType),
			DB2FieldValue(Data::geosetGroup),
			DB2FieldValue(Data::attachmentGeosetGroup),
			DB2FieldValue(Data::helmentGeosetVis)
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	using DFDB2ItemDisplayInfoMaterialResRecord = BFADB2ItemDisplayInfoMaterialResRecord;

	struct DFDB2ItemAppearanceRecord {
		struct Data {
			uint32_t id;
			uint32_t displayType;
			uint32_t itemDisplayInfoId;
			uint32_t defaultIconFileDataId;
			uint32_t uiOrder;
			uint32_t transmogPlayerConditionId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldValue(Data::displayType),
			DB2FieldValue(Data::itemDisplayInfoId),
			DB2FieldValue(Data::defaultIconFileDataId),
			DB2FieldValue(Data::uiOrder),
			DB2FieldValue(Data::transmogPlayerConditionId)

		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ItemModifiedAppearanceRecord {
		struct Data {
			uint32_t id;
			uint32_t itemId;
			uint32_t itemAppearanceModifierId;
			uint32_t itemAppearanceId;
			uint32_t orderIndex;
			uint32_t transmogSourceTypeEnum;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldValue(Data::id),				//inline id
			DB2FieldValue(Data::itemId),		// inline relation
			DB2FieldValue(Data::itemAppearanceModifierId),
			DB2FieldValue(Data::itemAppearanceId),
			DB2FieldValue(Data::orderIndex),
			DB2FieldValue(Data::transmogSourceTypeEnum)

		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ItemSparseRecord {
		struct Data {
			uint32_t id;
			uint64_t allowableRace;
			//inline strings
			//DB2InlineStringPlaceholder descriptionLang;
			//DB2InlineStringPlaceholder display3Lang;
			//DB2InlineStringPlaceholder display2Lang;
			//DB2InlineStringPlaceholder display1Lang;
			//DB2InlineStringPlaceholder displayLang;
			int32_t expansionID;
			float dmgVariance;
			int32_t limitCategory;
			uint32_t durationInInventory;
			float qualityModifier;
			uint32_t bagFamily;
			uint32_t startQuestId;
			uint32_t languageId;
			float itemRange;
			float statPercentageOfSocket[10];
			uint32_t statPercentEditor[10];
			uint32_t stackable;
			uint32_t maxCount;
			uint32_t minReputation;
			uint32_t requiredAbility;
			uint32_t sellPrice;
			uint32_t buyPrice;
			uint32_t vendorStackCount;
			float priceVariance;
			float priceRandomValue;
			uint32_t flags[4];
			uint32_t oppositeFactionItemId;
			uint32_t modifiedCraftingReagentId;
			uint32_t contentTuningId;
			uint32_t playerLevelToItemLevelCurveId;
			uint16_t itemNameDescriptionId;
			uint16_t requiredTransmogHoliday;
			uint16_t requiredHoliday;
			uint16_t gemProperties;
			uint16_t socketMatchEnchantmentId;
			uint16_t totemCategoryId;
			uint16_t instanceBound;
			uint16_t zoneBound[2];
			uint16_t itemSet;
			uint16_t lockId;
			uint16_t pageId;
			uint16_t itemDelay;
			uint16_t minFactionId;
			uint16_t requiredSkillRank;
			uint16_t requiredSkill;
			uint16_t itemLevel;
			uint16_t allowableClass;
			uint8_t artifactId;
			uint8_t spellWeight;
			uint8_t spellWeightCategory;
			uint8_t socketType[3];
			uint8_t sheatheType;
			uint8_t material;
			uint8_t pageMaterialId;
			uint8_t bonding;
			uint8_t damageType;
			uint8_t statModifierBonusStat[10];
			uint8_t containerSlots;
			uint8_t requiredPvpMedal;
			uint8_t requiredPvpRank;
			uint8_t requiredLevel;
			uint8_t inventoryType;
			uint8_t overallQualityId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldId(Data::id),
			DB2FieldValue(Data::allowableRace),
			DB2Field::inlineString(),
			DB2Field::inlineString(),
			DB2Field::inlineString(),
			DB2Field::inlineString(),
			DB2Field::inlineString(),
			DB2FieldValue(Data::expansionID),
			DB2FieldValue(Data::dmgVariance),
			DB2FieldValue(Data::limitCategory),
			DB2FieldValue(Data::durationInInventory),
			DB2FieldValue(Data::qualityModifier),
			DB2FieldValue(Data::bagFamily),
			DB2FieldValue(Data::startQuestId),
			DB2FieldValue(Data::languageId),
			DB2FieldValue(Data::itemRange),
			DB2FieldValue(Data::statPercentageOfSocket),
			DB2FieldValue(Data::statPercentEditor),
			DB2FieldValue(Data::stackable),
			DB2FieldValue(Data::maxCount),
			DB2FieldValue(Data::minReputation),
			DB2FieldValue(Data::requiredAbility),
			DB2FieldValue(Data::sellPrice),
			DB2FieldValue(Data::buyPrice),
			DB2FieldValue(Data::vendorStackCount),
			DB2FieldValue(Data::priceVariance),
			DB2FieldValue(Data::priceRandomValue),
			DB2FieldValue(Data::flags),
			DB2FieldValue(Data::oppositeFactionItemId),
			DB2FieldValue(Data::modifiedCraftingReagentId),
			DB2FieldValue(Data::contentTuningId),
			DB2FieldValue(Data::playerLevelToItemLevelCurveId),
			DB2FieldValue(Data::itemNameDescriptionId),
			DB2FieldValue(Data::requiredTransmogHoliday),
			DB2FieldValue(Data::requiredHoliday),
			DB2FieldValue(Data::gemProperties),
			DB2FieldValue(Data::socketMatchEnchantmentId),
			DB2FieldValue(Data::totemCategoryId),
			DB2FieldValue(Data::instanceBound),
			DB2FieldValue(Data::zoneBound),
			DB2FieldValue(Data::itemSet),
			DB2FieldValue(Data::lockId),
			DB2FieldValue(Data::pageId),
			DB2FieldValue(Data::itemDelay),
			DB2FieldValue(Data::minFactionId),
			DB2FieldValue(Data::requiredSkillRank),
			DB2FieldValue(Data::requiredSkill),
			DB2FieldValue(Data::itemLevel),
			DB2FieldValue(Data::allowableClass),
			DB2FieldValue(Data::artifactId),
			DB2FieldValue(Data::spellWeight),
			DB2FieldValue(Data::spellWeightCategory),
			DB2FieldValue(Data::socketType),
			DB2FieldValue(Data::sheatheType),
			DB2FieldValue(Data::material),
			DB2FieldValue(Data::pageMaterialId),
			DB2FieldValue(Data::bonding),
			DB2FieldValue(Data::damageType),
			DB2FieldValue(Data::statModifierBonusStat),
			DB2FieldValue(Data::containerSlots),
			DB2FieldValue(Data::requiredPvpMedal),
			DB2FieldValue(Data::requiredPvpRank),
			DB2FieldValue(Data::requiredLevel),
			DB2FieldValue(Data::inventoryType),
			DB2FieldValue(Data::overallQualityId)
		);

		DB2InlineString inlineStrings[schema.inlineStringCount()];

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	struct DFDB2ModelFileDataRecord {
		struct Data {
			float geoBox[6];
			uint32_t fileDataId;
			uint8_t flags;
			uint8_t lodCount;
			uint32_t modelResourcesId;
		} data;

		size_t recordIndex;

		constexpr static DB2Schema schema = DB2Schema(
			DB2FieldValue(Data::geoBox),
			DB2FieldValue(Data::fileDataId),	// inline ID
			DB2FieldValue(Data::flags),
			DB2FieldValue(Data::lodCount),
			DB2FieldValue(Data::modelResourcesId) // inline relation
		);

		static_assert(schema.recordSize() == sizeof(Data), "Schema size doesnt match data size.");
	};

	using DFDB2TextureFileDataRecord = BFADB2TextureFileDataRecord;


#pragma pack(pop)

}