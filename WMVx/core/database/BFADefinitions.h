/* Created via WDBReader schema_gen (client 8.3.7.35435) */
#pragma once
#include <WDBReader/Database/Schema.hpp>
#include <WDBReader/Database/DB2File.hpp>
#include <cstdint>


namespace core::db_bfa {

	using namespace WDBReader::Database;

#pragma pack(push, 1)

	struct AnimationDataRecord : public FixedRecord<AnimationDataRecord> {

		struct Data {
			uint32_t id;
			uint16_t fallback;
			uint8_t behaviorTier;
			uint32_t behaviorId;
			uint32_t flags[2];
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.fallback)>(),
			Field::value<decltype(data.behaviorTier)>(),
			Field::value<decltype(data.behaviorId)>(),
			Field::value<decltype(data.flags)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct CharacterFacialHairStylesRecord : public FixedRecord<CharacterFacialHairStylesRecord> {

		struct Data {
			uint32_t id;
			uint32_t geoset[5];
			uint8_t raceId;
			uint8_t sexId;
			uint8_t variationId;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.geoset)>(),
			Field::value<decltype(data.raceId)>(),
			Field::value<decltype(data.sexId)>(),
			Field::value<decltype(data.variationId)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct CharHairGeosetsRecord : public FixedRecord<CharHairGeosetsRecord> {

		struct Data {
			uint32_t id;
			uint8_t raceId;
			uint8_t sexId;
			uint8_t variationId;
			uint8_t geosetId;
			uint8_t showscalp;
			uint8_t variationType;
			uint8_t geosetType;
			uint8_t colorIndex;
			uint32_t customGeoFileDataId;
			uint32_t hdCustomGeoFileDataId;
			uint8_t field83032044010;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.raceId)>(Annotation().Relation()),
			Field::value<decltype(data.sexId)>(),
			Field::value<decltype(data.variationId)>(),
			Field::value<decltype(data.geosetId)>(),
			Field::value<decltype(data.showscalp)>(),
			Field::value<decltype(data.variationType)>(),
			Field::value<decltype(data.geosetType)>(),
			Field::value<decltype(data.colorIndex)>(),
			Field::value<decltype(data.customGeoFileDataId)>(),
			Field::value<decltype(data.hdCustomGeoFileDataId)>(),
			Field::value<decltype(data.field83032044010)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct ChrRacesRecord : public FixedRecord<ChrRacesRecord> {

		struct Data {
			string_data_t clientPrefix;
			string_data_t clientFileString;
			string_data_t nameLang;
			string_data_t nameFemaleLang;
			string_data_t nameLowercaseLang;
			string_data_t nameFemaleLowercaseLang;
			uint32_t id;
			uint32_t flags;
			uint32_t maleDisplayId;
			uint32_t femaleDisplayId;
			uint32_t highResMaleDisplayId;
			uint32_t highResFemaleDisplayId;
			uint32_t createScreenFileDataId;
			uint32_t selectScreenFileDataId;
			float maleCustomizeOffset[3];
			float femaleCustomizeOffset[3];
			uint32_t lowResScreenFileDataId;
			uint32_t alteredFormStartVisualKitId[3];
			uint32_t alteredFormFinishVisualKitId[3];
			uint32_t heritageArmorAchievementId;
			uint32_t startingLevel;
			uint32_t uiDisplayOrder;
			uint32_t femaleSkeletonFileDataId;
			uint32_t maleSkeletonFileDataId;
			uint32_t baseRaceId;
			uint32_t transmogrifyDisabledSlotMask;
			uint16_t factionId;
			uint16_t cinematicSequenceId;
			uint16_t resSicknessSpellId;
			uint16_t splashSoundId;
			uint8_t baseLanguage;
			uint8_t creatureType;
			uint8_t alliance;
			uint8_t raceRelated;
			uint8_t unalteredVisualRaceId;
			uint8_t charComponentTextureLayoutId;
			uint8_t charComponentTexLayoutHiResId;
			uint8_t defaultClassId;
			uint8_t neutralRaceId;
			uint8_t maleModelFallbackRaceId;
			uint8_t maleModelFallbackSex;
			uint8_t femaleModelFallbackRaceId;
			uint8_t femaleModelFallbackSex;
			uint8_t maleTextureFallbackRaceId;
			uint8_t maleTextureFallbackSex;
			uint8_t femaleTextureFallbackRaceId;
			uint8_t femaleTextureFallbackSex;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::string(),
			Field::string(),
			Field::langString(),
			Field::langString(),
			Field::langString(),
			Field::langString(),
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.maleDisplayId)>(),
			Field::value<decltype(data.femaleDisplayId)>(),
			Field::value<decltype(data.highResMaleDisplayId)>(),
			Field::value<decltype(data.highResFemaleDisplayId)>(),
			Field::value<decltype(data.createScreenFileDataId)>(),
			Field::value<decltype(data.selectScreenFileDataId)>(),
			Field::value<decltype(data.maleCustomizeOffset)>(),
			Field::value<decltype(data.femaleCustomizeOffset)>(),
			Field::value<decltype(data.lowResScreenFileDataId)>(),
			Field::value<decltype(data.alteredFormStartVisualKitId)>(),
			Field::value<decltype(data.alteredFormFinishVisualKitId)>(),
			Field::value<decltype(data.heritageArmorAchievementId)>(),
			Field::value<decltype(data.startingLevel)>(),
			Field::value<decltype(data.uiDisplayOrder)>(),
			Field::value<decltype(data.femaleSkeletonFileDataId)>(),
			Field::value<decltype(data.maleSkeletonFileDataId)>(),
			Field::value<decltype(data.baseRaceId)>(),
			Field::value<decltype(data.transmogrifyDisabledSlotMask)>(),
			Field::value<decltype(data.factionId)>(),
			Field::value<decltype(data.cinematicSequenceId)>(),
			Field::value<decltype(data.resSicknessSpellId)>(),
			Field::value<decltype(data.splashSoundId)>(),
			Field::value<decltype(data.baseLanguage)>(),
			Field::value<decltype(data.creatureType)>(),
			Field::value<decltype(data.alliance)>(),
			Field::value<decltype(data.raceRelated)>(),
			Field::value<decltype(data.unalteredVisualRaceId)>(),
			Field::value<decltype(data.charComponentTextureLayoutId)>(),
			Field::value<decltype(data.charComponentTexLayoutHiResId)>(),
			Field::value<decltype(data.defaultClassId)>(),
			Field::value<decltype(data.neutralRaceId)>(),
			Field::value<decltype(data.maleModelFallbackRaceId)>(),
			Field::value<decltype(data.maleModelFallbackSex)>(),
			Field::value<decltype(data.femaleModelFallbackRaceId)>(),
			Field::value<decltype(data.femaleModelFallbackSex)>(),
			Field::value<decltype(data.maleTextureFallbackRaceId)>(),
			Field::value<decltype(data.maleTextureFallbackSex)>(),
			Field::value<decltype(data.femaleTextureFallbackRaceId)>(),
			Field::value<decltype(data.femaleTextureFallbackSex)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct CharBaseSectionRecord : public FixedRecord<CharBaseSectionRecord> {

		struct Data {
			uint32_t id;
			uint8_t layoutResType;
			uint8_t variationEnum;
			uint8_t resolutionVariationEnum;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.layoutResType)>(),
			Field::value<decltype(data.variationEnum)>(),
			Field::value<decltype(data.resolutionVariationEnum)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct CharSectionsRecord : public FixedRecord<CharSectionsRecord> {

		struct Data {
			uint32_t id;
			uint8_t raceId;
			uint8_t sexId;
			uint8_t baseSection;
			uint8_t variationIndex;
			uint8_t colorIndex;
			uint16_t flags;
			uint32_t materialResourcesId[3];
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.raceId)>(),
			Field::value<decltype(data.sexId)>(),
			Field::value<decltype(data.baseSection)>(),
			Field::value<decltype(data.variationIndex)>(),
			Field::value<decltype(data.colorIndex)>(),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.materialResourcesId)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct CharComponentTextureSectionsRecord : public FixedRecord<CharComponentTextureSectionsRecord> {

		struct Data {
			uint32_t id;
			uint8_t charComponentTextureLayoutId;
			uint8_t sectionType;
			uint16_t x;
			uint16_t y;
			uint16_t width;
			uint16_t height;
			uint32_t overlapSectionMask;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.charComponentTextureLayoutId)>(),
			Field::value<decltype(data.sectionType)>(),
			Field::value<decltype(data.x)>(),
			Field::value<decltype(data.y)>(),
			Field::value<decltype(data.width)>(),
			Field::value<decltype(data.height)>(),
			Field::value<decltype(data.overlapSectionMask)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct CharComponentTextureLayoutsRecord : public FixedRecord<CharComponentTextureLayoutsRecord> {

		struct Data {
			uint32_t id;
			uint16_t width;
			uint16_t height;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.width)>(),
			Field::value<decltype(data.height)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct CreatureRecord : public FixedRecord<CreatureRecord> {

		struct Data {
			uint32_t id;
			string_data_t nameLang;
			string_data_t nameAltLang;
			string_data_t titleLang;
			string_data_t titleAltLang;
			uint8_t classification;
			uint8_t creatureType;
			uint16_t creatureFamily;
			uint8_t startAnimState;
			uint32_t displayId[4];
			float displayProbability[4];
			uint32_t alwaysItem[3];
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::langString(),
			Field::langString(),
			Field::langString(),
			Field::langString(),
			Field::value<decltype(data.classification)>(),
			Field::value<decltype(data.creatureType)>(),
			Field::value<decltype(data.creatureFamily)>(),
			Field::value<decltype(data.startAnimState)>(),
			Field::value<decltype(data.displayId)>(),
			Field::value<decltype(data.displayProbability)>(),
			Field::value<decltype(data.alwaysItem)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct CreatureModelDataRecord : public FixedRecord<CreatureModelDataRecord> {

		struct Data {
			uint32_t id;
			float geoBox[6];
			uint32_t flags;
			uint32_t fileDataId;
			uint32_t bloodId;
			uint32_t footprintTextureId;
			float footprintTextureLength;
			float footprintTextureWidth;
			float footprintParticleScale;
			uint32_t foleyMaterialId;
			uint32_t footstepCameraEffectId;
			uint32_t deathThudCameraEffectId;
			uint32_t soundId;
			uint32_t sizeClass;
			float collisionWidth;
			float collisionHeight;
			float worldEffectScale;
			uint32_t creatureGeosetDataId;
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
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.geoBox)>(),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.fileDataId)>(),
			Field::value<decltype(data.bloodId)>(),
			Field::value<decltype(data.footprintTextureId)>(),
			Field::value<decltype(data.footprintTextureLength)>(),
			Field::value<decltype(data.footprintTextureWidth)>(),
			Field::value<decltype(data.footprintParticleScale)>(),
			Field::value<decltype(data.foleyMaterialId)>(),
			Field::value<decltype(data.footstepCameraEffectId)>(),
			Field::value<decltype(data.deathThudCameraEffectId)>(),
			Field::value<decltype(data.soundId)>(),
			Field::value<decltype(data.sizeClass)>(),
			Field::value<decltype(data.collisionWidth)>(),
			Field::value<decltype(data.collisionHeight)>(),
			Field::value<decltype(data.worldEffectScale)>(),
			Field::value<decltype(data.creatureGeosetDataId)>(),
			Field::value<decltype(data.hoverHeight)>(),
			Field::value<decltype(data.attachedEffectScale)>(),
			Field::value<decltype(data.modelScale)>(),
			Field::value<decltype(data.missileCollisionRadius)>(),
			Field::value<decltype(data.missileCollisionPush)>(),
			Field::value<decltype(data.missileCollisionRaise)>(),
			Field::value<decltype(data.mountHeight)>(),
			Field::value<decltype(data.overrideLootEffectScale)>(),
			Field::value<decltype(data.overrideNameScale)>(),
			Field::value<decltype(data.overrideSelectionRadius)>(),
			Field::value<decltype(data.tamedPetBaseScale)>(),
			Field::value<decltype(data.mountScaleOtherIndex)>(),
			Field::value<decltype(data.mountScaleSelf)>(),
			Field::value<decltype(data.mountScaleOther)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct CreatureDisplayInfoRecord : public FixedRecord<CreatureDisplayInfoRecord> {

		struct Data {
			uint32_t id;
			uint16_t modelId;
			uint16_t soundId;
			uint8_t sizeClass;
			float creatureModelScale;
			uint8_t creatureModelAlpha;
			uint8_t bloodId;
			uint32_t extendedDisplayInfoId;
			uint16_t NPCSoundId;
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
			uint32_t textureVariationFileDataId[3];
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.modelId)>(),
			Field::value<decltype(data.soundId)>(),
			Field::value<decltype(data.sizeClass)>(),
			Field::value<decltype(data.creatureModelScale)>(),
			Field::value<decltype(data.creatureModelAlpha)>(),
			Field::value<decltype(data.bloodId)>(),
			Field::value<decltype(data.extendedDisplayInfoId)>(),
			Field::value<decltype(data.NPCSoundId)>(),
			Field::value<decltype(data.particleColorId)>(),
			Field::value<decltype(data.portraitCreatureDisplayInfoId)>(),
			Field::value<decltype(data.portraitTextureFileDataId)>(),
			Field::value<decltype(data.objectEffectPackageId)>(),
			Field::value<decltype(data.animReplacementSetId)>(),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.stateSpellVisualKitId)>(),
			Field::value<decltype(data.playerOverrideScale)>(),
			Field::value<decltype(data.petInstanceScale)>(),
			Field::value<decltype(data.unarmedWeaponType)>(),
			Field::value<decltype(data.mountPoofSpellVisualKitId)>(),
			Field::value<decltype(data.dissolveEffectId)>(),
			Field::value<decltype(data.gender)>(),
			Field::value<decltype(data.dissolveOutEffectId)>(),
			Field::value<decltype(data.creatureModelMinLod)>(),
			Field::value<decltype(data.textureVariationFileDataId)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};
	
	struct CreatureDisplayInfoExtraRecord : public FixedRecord<CreatureDisplayInfoExtraRecord> {

		struct Data {
			uint32_t id;
			uint8_t displayRaceId;
			uint8_t displaySexId;
			uint8_t displayClassId;
			uint8_t skinId;
			uint8_t faceId;
			uint8_t hairStyleId;
			uint8_t hairColorId;
			uint8_t facialHairId;
			uint8_t flags;
			uint32_t bakeMaterialResourcesId;
			uint32_t HDBakeMaterialResourcesId;
			uint8_t customDisplayOption[3];
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.displayRaceId)>(),
			Field::value<decltype(data.displaySexId)>(),
			Field::value<decltype(data.displayClassId)>(),
			Field::value<decltype(data.skinId)>(),
			Field::value<decltype(data.faceId)>(),
			Field::value<decltype(data.hairStyleId)>(),
			Field::value<decltype(data.hairColorId)>(),
			Field::value<decltype(data.facialHairId)>(),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.bakeMaterialResourcesId)>(),
			Field::value<decltype(data.HDBakeMaterialResourcesId)>(),
			Field::value<decltype(data.customDisplayOption)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct ItemRecord : public FixedRecord<ItemRecord> {

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
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.classId)>(),
			Field::value<decltype(data.subclassId)>(),
			Field::value<decltype(data.material)>(),
			Field::value<decltype(data.inventoryType)>(),
			Field::value<decltype(data.sheatheType)>(),
			Field::value<decltype(data.soundOverrideSubclassId)>(),
			Field::value<decltype(data.iconFileDataId)>(),
			Field::value<decltype(data.itemGroupSoundsId)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct ItemSparseRecord : public FixedRecord<ItemSparseRecord> {

		struct Data {
			uint32_t id;
			uint64_t allowableRace;
			string_data_t descriptionLang;
			string_data_t display3Lang;
			string_data_t display2Lang;
			string_data_t display1Lang;
			string_data_t displayLang;
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
			uint8_t requiredPVPMedal;
			uint8_t requiredPVPRank;
			uint8_t requiredLevel;
			uint8_t inventoryType;
			uint8_t overallQualityId;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.allowableRace)>(),
			Field::langString(),
			Field::langString(),
			Field::langString(),
			Field::langString(),
			Field::langString(),
			Field::value<decltype(data.dmgVariance)>(),
			Field::value<decltype(data.durationInInventory)>(),
			Field::value<decltype(data.qualityModifier)>(),
			Field::value<decltype(data.bagFamily)>(),
			Field::value<decltype(data.itemRange)>(),
			Field::value<decltype(data.statPercentageOfSocket)>(),
			Field::value<decltype(data.statPercentEditor)>(),
			Field::value<decltype(data.stackable)>(),
			Field::value<decltype(data.maxCount)>(),
			Field::value<decltype(data.requiredAbility)>(),
			Field::value<decltype(data.sellPrice)>(),
			Field::value<decltype(data.buyPrice)>(),
			Field::value<decltype(data.vendorStackCount)>(),
			Field::value<decltype(data.priceVariance)>(),
			Field::value<decltype(data.priceRandomValue)>(),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.oppositeFactionItemId)>(),
			Field::value<decltype(data.itemNameDescriptionId)>(),
			Field::value<decltype(data.requiredTransmogHoliday)>(),
			Field::value<decltype(data.requiredHoliday)>(),
			Field::value<decltype(data.limitCategory)>(),
			Field::value<decltype(data.gemProperties)>(),
			Field::value<decltype(data.socketMatchEnchantmentId)>(),
			Field::value<decltype(data.totemCategoryId)>(),
			Field::value<decltype(data.instanceBound)>(),
			Field::value<decltype(data.zoneBound)>(),
			Field::value<decltype(data.itemSet)>(),
			Field::value<decltype(data.lockId)>(),
			Field::value<decltype(data.startQuestId)>(),
			Field::value<decltype(data.pageId)>(),
			Field::value<decltype(data.itemDelay)>(),
			Field::value<decltype(data.scalingStatDistributionId)>(),
			Field::value<decltype(data.minFactionId)>(),
			Field::value<decltype(data.requiredSkillRank)>(),
			Field::value<decltype(data.requiredSkill)>(),
			Field::value<decltype(data.itemLevel)>(),
			Field::value<decltype(data.allowableClass)>(),
			Field::value<decltype(data.expansionId)>(),
			Field::value<decltype(data.artifactId)>(),
			Field::value<decltype(data.spellWeight)>(),
			Field::value<decltype(data.spellWeightCategory)>(),
			Field::value<decltype(data.socketType)>(),
			Field::value<decltype(data.sheatheType)>(),
			Field::value<decltype(data.material)>(),
			Field::value<decltype(data.pageMaterialId)>(),
			Field::value<decltype(data.languageId)>(),
			Field::value<decltype(data.bonding)>(),
			Field::value<decltype(data.damageType)>(),
			Field::value<decltype(data.statModifierBonusStat)>(),
			Field::value<decltype(data.containerSlots)>(),
			Field::value<decltype(data.minReputation)>(),
			Field::value<decltype(data.requiredPVPMedal)>(),
			Field::value<decltype(data.requiredPVPRank)>(),
			Field::value<decltype(data.requiredLevel)>(),
			Field::value<decltype(data.inventoryType)>(),
			Field::value<decltype(data.overallQualityId)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct ItemDisplayInfoRecord : public FixedRecord<ItemDisplayInfoRecord> {

		struct Data {
			uint32_t id;
			uint32_t itemVisual;
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
			uint32_t helmetGeosetVis[2];
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.itemVisual)>(),
			Field::value<decltype(data.particleColorId)>(),
			Field::value<decltype(data.itemRangedDisplayInfoId)>(),
			Field::value<decltype(data.overrideSwooshSoundKitId)>(),
			Field::value<decltype(data.sheatheTransformMatrixId)>(),
			Field::value<decltype(data.stateSpellVisualKitId)>(),
			Field::value<decltype(data.sheathedSpellVisualKitId)>(),
			Field::value<decltype(data.unsheathedSpellVisualKitId)>(),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.modelResourcesId)>(),
			Field::value<decltype(data.modelMaterialResourcesId)>(),
			Field::value<decltype(data.modelType)>(),
			Field::value<decltype(data.geosetGroup)>(),
			Field::value<decltype(data.attachmentGeosetGroup)>(),
			Field::value<decltype(data.helmetGeosetVis)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct ItemAppearanceRecord : public FixedRecord<ItemAppearanceRecord> {

		struct Data {
			uint32_t id;
			uint8_t displayType;
			uint32_t itemDisplayInfoId;
			uint32_t defaultIconFileDataId;
			uint32_t uiOrder;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.displayType)>(),
			Field::value<decltype(data.itemDisplayInfoId)>(),
			Field::value<decltype(data.defaultIconFileDataId)>(),
			Field::value<decltype(data.uiOrder)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct ItemModifiedAppearanceRecord : public FixedRecord<ItemModifiedAppearanceRecord> {

		struct Data {
			uint32_t id;
			uint32_t itemId;
			uint8_t itemAppearanceModifierId;
			uint16_t itemAppearanceId;
			uint8_t orderIndex;
			uint8_t transmogSourceTypeEnum;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.itemId)>(Annotation().Relation()),
			Field::value<decltype(data.itemAppearanceModifierId)>(),
			Field::value<decltype(data.itemAppearanceId)>(),
			Field::value<decltype(data.orderIndex)>(),
			Field::value<decltype(data.transmogSourceTypeEnum)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct ItemDisplayInfoMaterialResRecord : public FixedRecord<ItemDisplayInfoMaterialResRecord> {

		struct Data {
			uint32_t id;
			uint8_t componentSection;
			uint32_t materialResourcesId;
			uint32_t itemDisplayInfoId;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.componentSection)>(),
			Field::value<decltype(data.materialResourcesId)>(),
			Field::value<decltype(data.itemDisplayInfoId)>(Annotation().NonInline().Relation())
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct GuildTabardBackgroundRecord : public FixedRecord<GuildTabardBackgroundRecord> {

		struct Data {
			uint32_t id;
			uint32_t tier;
			uint32_t component;
			uint32_t fileDataId;
			uint32_t color;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.tier)>(),
			Field::value<decltype(data.component)>(),
			Field::value<decltype(data.fileDataId)>(),
			Field::value<decltype(data.color)>(Annotation().NonInline().Relation())
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct GuildTabardBorderRecord : public FixedRecord<GuildTabardBorderRecord> {

		struct Data {
			uint32_t id;
			uint32_t borderId;
			uint32_t tier;
			uint32_t component;
			uint32_t fileDataId;
			uint32_t color;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.borderId)>(),
			Field::value<decltype(data.tier)>(),
			Field::value<decltype(data.component)>(),
			Field::value<decltype(data.fileDataId)>(),
			Field::value<decltype(data.color)>(Annotation().NonInline().Relation())
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct GuildTabardEmblemRecord : public FixedRecord<GuildTabardEmblemRecord> {

		struct Data {
			uint32_t id;
			uint32_t component;
			uint32_t color;
			uint32_t fileDataId;
			uint32_t emblemId;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.component)>(),
			Field::value<decltype(data.color)>(),
			Field::value<decltype(data.fileDataId)>(),
			Field::value<decltype(data.emblemId)>(Annotation().NonInline().Relation())
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct ModelFileDataRecord : public FixedRecord<ModelFileDataRecord> {

		struct Data {
			uint32_t fileDataId;
			uint8_t flags;
			uint8_t lodCount;
			uint32_t modelResourcesId;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.fileDataId)>(Annotation().Id()),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.lodCount)>(),
			Field::value<decltype(data.modelResourcesId)>(Annotation().Relation())
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct TextureFileDataRecord : public FixedRecord<TextureFileDataRecord> {

		struct Data {
			uint32_t fileDataId;
			uint8_t usageType;
			uint32_t materialResourcesId;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.fileDataId)>(Annotation().Id()),
			Field::value<decltype(data.usageType)>(),
			Field::value<decltype(data.materialResourcesId)>(Annotation().Relation())
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct ComponentModelFileDataRecord : public FixedRecord<ComponentModelFileDataRecord> {

		struct Data {
			uint32_t id;
			uint8_t genderIndex;
			uint8_t classId;
			uint8_t raceId;
			uint8_t positionIndex;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.genderIndex)>(),
			Field::value<decltype(data.classId)>(),
			Field::value<decltype(data.raceId)>(),
			Field::value<decltype(data.positionIndex)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

	struct ComponentTextureFileDataRecord : public FixedRecord<ComponentTextureFileDataRecord> {

		struct Data {
			uint32_t id;
			uint8_t genderIndex;
			uint8_t classId;
			uint8_t raceId;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id().NonInline()),
			Field::value<decltype(data.genderIndex)>(),
			Field::value<decltype(data.classId)>(),
			Field::value<decltype(data.raceId)>()
		);

		static_assert(DB2Format::recordSizeDest(schema) == sizeof(data));
	};

#pragma pack(pop)

}