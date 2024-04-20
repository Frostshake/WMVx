#pragma once

#include <cstdint>
#include <array>
#include <map>
#include "../game/GameConstants.h"
#include "../filesystem/GameFileSystem.h"
#include "../modeling/M2Defintions.h"

namespace core {

	struct CharacterRelationSearchContext {
	public:

		int8_t gender;
		uint32_t race;
		int8_t fallbackGender;
		uint32_t fallbackRace;
		QString raceClientPrefix;
		QString fallbackRaceClientPrefix;

		static const int8_t MODERN_GENDER_IGNORE = -1;

		CharacterRelationSearchContext() = default;

		inline static CharacterRelationSearchContext makeLegacy(auto gender_, auto race_, QString racePrefix_) {
			return make(gender_, race_, (int8_t)0, (uint32_t)0, racePrefix_);
		}

		inline static CharacterRelationSearchContext makeModern(auto gender_, auto race_, auto fallbackGender_, auto fallbackRace_) {
			return make(gender_, race_, fallbackGender_, fallbackRace_, "");
		}

		inline static CharacterRelationSearchContext make(auto gender_, auto race_, auto fallbackGender_, auto fallbackRace_, QString racePrefix_, QString fallbackRacePrefix_ = "") {
			static_assert(sizeof(gender) >= sizeof(gender_));
			static_assert(sizeof(race) >= sizeof(race_));
			static_assert(sizeof(fallbackGender) >= sizeof(fallbackGender_));
			static_assert(sizeof(fallbackRace) >= sizeof(fallbackRace_));
				
			CharacterRelationSearchContext result;
			result.gender =			(decltype(gender))gender_;
			result.race =			(decltype(race))race_;
			result.fallbackGender = (decltype(fallbackGender))fallbackGender_;
			result.fallbackRace =	(decltype(fallbackRace))fallbackRace_;
			result.raceClientPrefix = racePrefix_;
			result.fallbackRaceClientPrefix = fallbackRacePrefix_;
			return result;
		}

	};

	class AnimationDataRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		virtual QString getName() const = 0;
	};

	class CharacterRaceRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		virtual QString getClientPrefix() const = 0;

		virtual QString getClientFileString() const = 0;

		virtual std::optional<uint32_t> getComponentTextureLayoutId(bool hd) const = 0;

		virtual std::optional<CharacterRelationSearchContext> getModelSearchContext(Gender gender) const = 0;

		virtual std::optional<CharacterRelationSearchContext> getTextureSearchContext(Gender gender) const = 0;
	};

	class CharacterFacialHairStyleRecordAdaptor {
	public:
		constexpr virtual uint32_t getRaceId() const = 0;

		constexpr virtual Gender getSexId() const = 0;

		constexpr virtual uint32_t getGeoset100() const = 0;

		constexpr virtual uint32_t getGeoset200() const = 0;

		constexpr virtual uint32_t getGeoset300() const = 0;

	};

	class CharacterHairGeosetRecordAdaptor {
	public:
		constexpr virtual uint32_t getRaceId() const = 0;

		constexpr virtual Gender getSexId() const = 0;

		constexpr virtual uint32_t getGeoset() const = 0;

		constexpr virtual bool isBald() const = 0;
	};

	class CharacterSectionRecordAdaptor {
	public:

		constexpr virtual uint32_t getId() const = 0;

		constexpr virtual uint32_t getRaceId() const = 0;

		constexpr virtual Gender getSexId() const = 0;

		constexpr virtual CharacterSectionType getType() const = 0;

		virtual std::array<GameFileUri, 3> getTextures() const = 0;

		constexpr virtual uint32_t getSection() const = 0;

		constexpr virtual uint32_t getVariationIndex() const = 0;

		constexpr virtual bool isHD() const = 0;
	};

	class CharacterComponentTextureAdaptor {
	public:
		constexpr virtual uint32_t getLayoutId() const = 0;

		constexpr virtual int32_t getLayoutWidth() const = 0;

		constexpr virtual int32_t getLayoutHeight() const = 0;

		virtual std::map<CharacterRegion, CharacterRegionCoords> getRegions() const = 0;
	};

	class CreatureModelDataRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		virtual GameFileUri getModelUri() const = 0;
	};

	class CreatureDisplayExtraRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		constexpr virtual uint32_t getRaceId() const = 0;

		constexpr virtual Gender getSexId() const = 0;

		constexpr virtual uint32_t getSkinId() const = 0;

		constexpr virtual uint32_t getFaceId() const = 0;
		
		constexpr virtual uint32_t getHairStyleId() const = 0;

		constexpr virtual uint32_t getHairColorId() const = 0;

		constexpr virtual uint32_t getFacialHairId() const = 0;

		virtual std::map<ItemInventorySlotId, uint32_t> getItemDisplayIds() const = 0;
	};

	class CreatureDisplayRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		constexpr virtual uint32_t getModelId() const = 0;

		virtual std::array<GameFileUri, 3> getTextures() const = 0;

		virtual const CreatureDisplayExtraRecordAdaptor* getExtra() const = 0;
	};

	class ItemRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		constexpr virtual uint32_t getItemDisplayInfoId() const = 0;

		constexpr virtual ItemInventorySlotId getInventorySlotId() const = 0;

		constexpr virtual SheathTypes getSheatheTypeId() const = 0;

		constexpr virtual ItemQualityId getItemQuality() const = 0;

		virtual QString getName() const = 0;
	};

	class ItemDisplayRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		virtual std::array<GameFileUri,2> getModel(CharacterSlot char_slot, ItemInventorySlotId item_slot, const std::optional<CharacterRelationSearchContext>& search) const = 0;

		constexpr virtual uint32_t getGeosetGlovesFlags() const = 0;

		constexpr virtual uint32_t getGeosetBracerFlags() const = 0;

		constexpr virtual uint32_t getGeosetRobeFlags() const = 0;

		virtual std::array<GameFileUri, 2> getModelTexture(CharacterSlot char_slot, ItemInventorySlotId item_slot, const std::optional<CharacterRelationSearchContext>& search) const = 0;

		virtual GameFileUri getTextureUpperArm() const = 0;

		virtual GameFileUri getTextureLowerArm() const = 0;

		virtual GameFileUri getTextureHands() const = 0;

		virtual GameFileUri getTextureUpperChest() const = 0;

		virtual GameFileUri getTextureLowerChest() const = 0;

		virtual GameFileUri getTextureUpperLeg() const = 0;

		virtual GameFileUri getTextureLowerLeg() const = 0;

		virtual GameFileUri getTextureFoot() const = 0;

		constexpr virtual uint32_t getItemVisualId() const = 0;
	};

	class ItemVisualRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		virtual std::array<uint32_t, 5> getItemVisualEffectIds() const = 0;
	};

	class ItemVisualEffectRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		virtual QString getModel() const = 0;
	};

	class SpellItemEnchantmentRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		virtual QString getName() const = 0;

		constexpr virtual uint32_t getItemVisualId() const = 0;
	};

	/* --- */

	class NPCRecordAdaptor {
	public:
		constexpr virtual uint32_t getId() const = 0;

		constexpr virtual uint32_t getModelId() const = 0;

		constexpr virtual uint32_t getType() const = 0;

		virtual QString getName() const = 0;
	};
}