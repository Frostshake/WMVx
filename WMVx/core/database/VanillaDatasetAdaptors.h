#pragma once

#include "GameDatasetAdaptors.h"
#include "GenericDBCDatasetAdaptors.h"
#include "ReferenceSource.h"
#include "VanillaDefinitions.h"


namespace core {

	using VanillaAnimationDataRecordAdaptor = GenericLegacyDBCAnimationDataRecordAdaptor<db_vanilla::AnimationDataRecord>;
	using VanillaChrRacesRecordAdaptor = GenericLegacyDBCChrRacesRecordAdaptor<db_vanilla::ChrRacesRecord>;
	using VanillaCharSectionsRecordAdaptor = GenericLegacyDBCCharSectionsRecordAdaptor<db_vanilla::CharSectionsRecord>;

	class VanillaCharacterFacialHairStylesRecordAdaptor : public CharacterFacialHairStyleRecordAdaptor, public GenericDBCRecordAdaptor<db_vanilla::CharacterFacialHairStylesRecord> {
	public:
		using GenericDBCRecordAdaptor<db_vanilla::CharacterFacialHairStylesRecord>::GenericDBCRecordAdaptor;

		constexpr uint32_t getRaceId() const override {
			return this->_record.data.raceId;
		}

		constexpr Gender getSexId() const override {
			return static_cast<Gender>(this->_record.data.sexId);
		}


		constexpr virtual uint32_t getGeoset100() const override {
			return this->_record.data.geoset[3];
		}

		constexpr virtual uint32_t getGeoset200() const override {
			return this->_record.data.geoset[5];
		}

		constexpr virtual uint32_t getGeoset300() const  override {
			return this->_record.data.geoset[4];
		}
	};

	using VanillaCharHairGeosetsRecordAdaptor = GenericLegacyDBCCharacterHairGeosetsRecordAdaptor<db_vanilla::CharHairGeosetsRecord>;
	using VanillaCreatureModelDataRecordAdaptor = GenericLegacyDBCCreatureModelDataRecordAdaptor<db_vanilla::CreatureModelDataRecord>;
	using VanillaCreatureModelDisplayInfoExtraRecordAdaptor = GenericLegacyDBCCreatureDisplayExtraRecordAdaptor<db_vanilla::CreatureDisplayInfoExtraRecord>;
	using VanillaCreatureDisplayInfoRecordAdaptor = GenericLegacyDBCCreatureDisplayRecordAdaptor<db_vanilla::CreatureDisplayInfoRecord>;

	class VanillaItemRecordAdaptor : public ItemRecordAdaptor {
	public:
		VanillaItemRecordAdaptor(const ReferenceSourceItemsCache::ItemCacheRecord* cache_record) 
			: cacheRecord(cache_record) {}
		VanillaItemRecordAdaptor(VanillaItemRecordAdaptor&&) = default;
		virtual ~VanillaItemRecordAdaptor() = default;

		constexpr uint32_t getId() const override {
			return cacheRecord->itemId;
		}

		constexpr std::vector<uint32_t> getItemDisplayInfoId() const override {
			return { cacheRecord->itemDisplayInfoId };
		}

		constexpr ItemInventorySlotId getInventorySlotId() const override {
			return cacheRecord->inventorySlot;
		}

		constexpr SheathTypes getSheatheTypeId() const override {
			return (SheathTypes)cacheRecord->sheatheTypeId;
		}

		constexpr ItemQualityId getItemQuality() const override {
			return cacheRecord->itemQuality;
		}

		QString getName() const override {
			return cacheRecord->name;
		}

	protected:
		const ReferenceSourceItemsCache::ItemCacheRecord* cacheRecord;
	};

	using VanillaItemDisplayInfoRecordAdaptor = GenericLegacyDBCItemDisplayInfoRecordAdaptor<db_vanilla::ItemDisplayInfoRecord>;
	using VanillaItemVisualRecordAdaptor = GenericLegacyDBCItemVisualRecordAdaptor<db_vanilla::ItemVisualsRecord>;
	using VanillaItemVisualEffectRecordAdaptor = GenericLegacyDBCItemVisualEffectRecordAdaptor<db_vanilla::ItemVisualEffectsRecord>;
	using VanillaSpellItemEnchantmentRecordAdaptor = GenericLegacyDBCSpellItemEnchantmentRecordAdaptor<db_vanilla::SpellItemEnchantmentRecord>;

}