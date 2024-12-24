/* Created via WDBReader schema_gen (client 2.4.3.8606) */
#pragma once
#include <WDBReader/Database/Schema.hpp>
#include <WDBReader/Database/DBCFile.hpp>
#include <cstdint>

namespace core::db_tbc {

	using namespace WDBReader::Database;

#pragma pack(push, 1)

	struct AnimationDataRecord : public FixedRecord<AnimationDataRecord> {

		struct Data {
			uint32_t id;
			string_data_t name;
			uint32_t weaponflags;
			uint32_t bodyflags;
			uint32_t field0703694004;
			uint32_t flags;
			uint32_t fallback;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::string(),
			Field::value<decltype(data.weaponflags)>(),
			Field::value<decltype(data.bodyflags)>(),
			Field::value<decltype(data.field0703694004)>(),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.fallback)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct CharacterFacialHairStylesRecord : public FixedRecord<CharacterFacialHairStylesRecord> {

		struct Data {
			uint32_t raceId;
			uint32_t sexId;
			uint32_t variationId;
			uint32_t geoset[8];
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.raceId)>(),
			Field::value<decltype(data.sexId)>(),
			Field::value<decltype(data.variationId)>(),
			Field::value<decltype(data.geoset)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct CharHairGeosetsRecord : public FixedRecord<CharHairGeosetsRecord> {

		struct Data {
			uint32_t id;
			uint32_t raceId;
			uint32_t sexId;
			uint32_t variationId;
			uint32_t geosetId;
			uint32_t showscalp;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.raceId)>(),
			Field::value<decltype(data.sexId)>(),
			Field::value<decltype(data.variationId)>(),
			Field::value<decltype(data.geosetId)>(),
			Field::value<decltype(data.showscalp)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct CharSectionsRecord : public FixedRecord<CharSectionsRecord> {

		struct Data {
			uint32_t id;
			uint32_t raceId;
			uint32_t sexId;
			uint32_t baseSection;
			uint32_t variationIndex;
			uint32_t colorIndex;
			string_data_t textureName[3];
			uint32_t flags;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.raceId)>(),
			Field::value<decltype(data.sexId)>(),
			Field::value<decltype(data.baseSection)>(),
			Field::value<decltype(data.variationIndex)>(),
			Field::value<decltype(data.colorIndex)>(),
			Field::string(3),
			Field::value<decltype(data.flags)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct ChrRacesRecord : public FixedRecord<ChrRacesRecord> {

		struct Data {
			uint32_t id;
			uint32_t flags;
			uint32_t factionId;
			uint32_t explorationSoundId;
			uint32_t maleDisplayId;
			uint32_t femaleDisplayId;
			string_data_t clientPrefix;
			float mountScale;
			uint32_t baseLanguage;
			uint32_t creatureType;
			uint32_t resSicknessSpellId;
			uint32_t splashSoundId;
			string_data_t clientFileString;
			uint32_t cinematicSequenceId;
			DBCLangString<DBCVersion::BC_WOTLK> nameLang;
			DBCLangString<DBCVersion::BC_WOTLK> nameFemaleLang;
			DBCLangString<DBCVersion::BC_WOTLK> nameMaleLang;
			string_data_t facialHairCustomization[2];
			string_data_t hairCustomization;
			uint32_t requiredExpansion;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.factionId)>(),
			Field::value<decltype(data.explorationSoundId)>(),
			Field::value<decltype(data.maleDisplayId)>(),
			Field::value<decltype(data.femaleDisplayId)>(),
			Field::string(),
			Field::value<decltype(data.mountScale)>(),
			Field::value<decltype(data.baseLanguage)>(),
			Field::value<decltype(data.creatureType)>(),
			Field::value<decltype(data.resSicknessSpellId)>(),
			Field::value<decltype(data.splashSoundId)>(),
			Field::string(),
			Field::value<decltype(data.cinematicSequenceId)>(),
			Field::langString(),
			Field::langString(),
			Field::langString(),
			Field::string(2),
			Field::string(),
			Field::value<decltype(data.requiredExpansion)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};


	struct CreatureDisplayInfoRecord : public FixedRecord<CreatureDisplayInfoRecord> {

		struct Data {
			uint32_t id;
			uint32_t modelId;
			uint32_t soundId;
			uint32_t extendedDisplayInfoId;
			float creatureModelScale;
			uint32_t creatureModelAlpha;
			string_data_t textureVariation[3];
			string_data_t portraitTextureName;
			uint32_t sizeClass;
			uint32_t bloodId;
			uint32_t NPCSoundId;
			uint32_t particleColorId;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.modelId)>(),
			Field::value<decltype(data.soundId)>(),
			Field::value<decltype(data.extendedDisplayInfoId)>(),
			Field::value<decltype(data.creatureModelScale)>(),
			Field::value<decltype(data.creatureModelAlpha)>(),
			Field::string(3),
			Field::string(),
			Field::value<decltype(data.sizeClass)>(),
			Field::value<decltype(data.bloodId)>(),
			Field::value<decltype(data.NPCSoundId)>(),
			Field::value<decltype(data.particleColorId)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct CreatureDisplayInfoExtraRecord : public FixedRecord<CreatureDisplayInfoExtraRecord> {

		struct Data {
			uint32_t id;
			uint32_t displayRaceId;
			uint32_t displaySexId;
			uint32_t skinId;
			uint32_t faceId;
			uint32_t hairStyleId;
			uint32_t hairColorId;
			uint32_t facialHairId;
			uint32_t NPCItemDisplayId[11];
			uint32_t flags;
			string_data_t bakeName;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.displayRaceId)>(),
			Field::value<decltype(data.displaySexId)>(),
			Field::value<decltype(data.skinId)>(),
			Field::value<decltype(data.faceId)>(),
			Field::value<decltype(data.hairStyleId)>(),
			Field::value<decltype(data.hairColorId)>(),
			Field::value<decltype(data.facialHairId)>(),
			Field::value<decltype(data.NPCItemDisplayId)>(),
			Field::value<decltype(data.flags)>(),
			Field::string()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct CreatureModelDataRecord : public FixedRecord<CreatureModelDataRecord> {

		struct Data {
			uint32_t id;
			uint32_t flags;
			string_data_t modelName;
			uint32_t sizeClass;
			float modelScale;
			uint32_t bloodId;
			uint32_t footprintTextureId;
			float footprintTextureLength;
			float footprintTextureWidth;
			float footprintParticleScale;
			uint32_t foleyMaterialId;
			uint32_t footstepShakeSize;
			uint32_t deathThudShakeSize;
			uint32_t soundId;
			float collisionWidth;
			float collisionHeight;
			float mountHeight;
			float geoBox[6];
			float attachedEffectScale;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.flags)>(),
			Field::string(),
			Field::value<decltype(data.sizeClass)>(),
			Field::value<decltype(data.modelScale)>(),
			Field::value<decltype(data.bloodId)>(),
			Field::value<decltype(data.footprintTextureId)>(),
			Field::value<decltype(data.footprintTextureLength)>(),
			Field::value<decltype(data.footprintTextureWidth)>(),
			Field::value<decltype(data.footprintParticleScale)>(),
			Field::value<decltype(data.foleyMaterialId)>(),
			Field::value<decltype(data.footstepShakeSize)>(),
			Field::value<decltype(data.deathThudShakeSize)>(),
			Field::value<decltype(data.soundId)>(),
			Field::value<decltype(data.collisionWidth)>(),
			Field::value<decltype(data.collisionHeight)>(),
			Field::value<decltype(data.mountHeight)>(),
			Field::value<decltype(data.geoBox)>(),
			Field::value<decltype(data.attachedEffectScale)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct ItemRecord : public FixedRecord<ItemRecord> {

		struct Data {
			uint32_t id;
			uint32_t displayInfoId;
			uint32_t inventoryType;
			uint32_t sheatheType;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.displayInfoId)>(),
			Field::value<decltype(data.inventoryType)>(),
			Field::value<decltype(data.sheatheType)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct ItemDisplayInfoRecord : public FixedRecord<ItemDisplayInfoRecord> {

		struct Data {
			uint32_t id;
			string_data_t modelName[2];
			string_data_t modelTexture[2];
			string_data_t inventoryIcon[2];
			uint32_t geosetGroup[3];
			uint32_t flags;
			uint32_t spellVisualId;
			uint32_t groupSoundIndex;
			uint32_t helmetGeosetVisId[2];
			string_data_t texture[8];
			uint32_t itemVisualId;
			uint32_t particleColorId;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::string(2),
			Field::string(2),
			Field::string(2),
			Field::value<decltype(data.geosetGroup)>(),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.spellVisualId)>(),
			Field::value<decltype(data.groupSoundIndex)>(),
			Field::value<decltype(data.helmetGeosetVisId)>(),
			Field::string(8),
			Field::value<decltype(data.itemVisualId)>(),
			Field::value<decltype(data.particleColorId)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct ItemVisualsRecord : public FixedRecord<ItemVisualsRecord> {

		struct Data {
			uint32_t id;
			uint32_t slot[5];
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.slot)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct ItemVisualEffectsRecord : public FixedRecord<ItemVisualEffectsRecord> {

		struct Data {
			uint32_t id;
			string_data_t model;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::string()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

	struct SpellItemEnchantmentRecord : public FixedRecord<SpellItemEnchantmentRecord> {

		struct Data {
			uint32_t id;
			uint32_t effect[3];
			uint32_t effectPointsMin[3];
			uint32_t effectPointsMax[3];
			uint32_t effectArg[3];
			DBCLangString<DBCVersion::BC_WOTLK> nameLang;
			uint32_t itemVisual;
			uint32_t flags;
			uint32_t srcItemId;
			uint32_t conditionId;
		} data;

		size_t recordIndex;
		RecordEncryption encryptionState;

		constexpr static Schema schema = Schema(
			Field::value<decltype(data.id)>(Annotation().Id()),
			Field::value<decltype(data.effect)>(),
			Field::value<decltype(data.effectPointsMin)>(),
			Field::value<decltype(data.effectPointsMax)>(),
			Field::value<decltype(data.effectArg)>(),
			Field::langString(),
			Field::value<decltype(data.itemVisual)>(),
			Field::value<decltype(data.flags)>(),
			Field::value<decltype(data.srcItemId)>(),
			Field::value<decltype(data.conditionId)>()
		);

		static_assert(DBCFormat::recordSizeDest(schema, DBCVersion::BC_WOTLK) == sizeof(data));
	};

#pragma pack(pop)

}