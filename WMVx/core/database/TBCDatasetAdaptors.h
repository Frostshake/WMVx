#pragma once
#include "GameDatasetAdaptors.h"
#include "GenericDBCDatasetAdaptors.h"
#include "ReferenceSource.h"
#include "TBCDefinitions.h"

namespace core {


	using TBCAnimationDataRecordAdaptor = GenericLegacyDBCAnimationDataRecordAdaptor<db_tbc::AnimationDataRecord>;
	using TBCChrRacesRecordAdaptor = GenericLegacyDBCChrRacesRecordAdaptor <db_tbc::ChrRacesRecord >;
	using TBCCharSectionsRecordAdaptor = GenericLegacyDBCCharSectionsRecordAdaptor<db_tbc::CharSectionsRecord>;

	class TBCCharacterFacialHairStylesRecordAdaptor : public CharacterFacialHairStyleRecordAdaptor, public GenericDBCRecordAdaptor<db_tbc::CharacterFacialHairStylesRecord> {
	public:
		using GenericDBCRecordAdaptor<db_tbc::CharacterFacialHairStylesRecord>::GenericDBCRecordAdaptor;

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

	using TBCCharHairGeosetsRecordAdaptor = GenericLegacyDBCCharacterHairGeosetsRecordAdaptor<db_tbc::CharHairGeosetsRecord>;
	using TBCCreatureModelDataRecordAdaptor = GenericLegacyDBCCreatureModelDataRecordAdaptor<db_tbc::CreatureModelDataRecord>;
	using TBCCreatureModelDisplayInfoExtraRecordAdaptor = GenericLegacyDBCCreatureDisplayExtraRecordAdaptor<db_tbc::CreatureDisplayInfoExtraRecord>;
	using TBCCreatureDisplayInfoRecordAdaptor = GenericLegacyDBCCreatureDisplayRecordAdaptor<db_tbc::CreatureDisplayInfoRecord>;

	class TBCItemRecordAdaptor : public ItemRecordAdaptor, public GenericDBCRecordAdaptor<db_tbc::ItemRecord> {
	public:
		TBCItemRecordAdaptor(db_tbc::ItemRecord&& record, const ReferenceSourceItemsCache::ItemCacheRecord* cache_record) :
			GenericDBCRecordAdaptor<db_tbc::ItemRecord>(std::move(record)), cacheRecord(cache_record)
		{}
		TBCItemRecordAdaptor(TBCItemRecordAdaptor&&) = default;
		virtual ~TBCItemRecordAdaptor() = default;

		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		constexpr uint32_t getItemDisplayInfoId() const override {
			return this->_record.data.displayInfoId;
		}

		constexpr ItemInventorySlotId getInventorySlotId() const override {
			return (ItemInventorySlotId)this->_record.data.inventoryType;
		}

		constexpr SheathTypes getSheatheTypeId() const override {
			return (SheathTypes)this->_record.data.sheatheType;
		}

		constexpr ItemQualityId getItemQuality() const override {
			return cacheRecord == nullptr ? ItemQualityId::POOR : cacheRecord->itemQuality;
		}

		QString getName() const override {
			return cacheRecord == nullptr ? "Unknown (Missing Name)" : cacheRecord->name;
		}

	protected:
		const ReferenceSourceItemsCache::ItemCacheRecord* cacheRecord;
	};

	using TBCItemDisplayInfoRecordAdaptor = GenericLegacyDBCItemDisplayInfoRecordAdaptor<db_tbc::ItemDisplayInfoRecord>;
	using TBCItemVisualRecordAdaptor = GenericLegacyDBCItemVisualRecordAdaptor<db_tbc::ItemVisualsRecord>;
	using TBCItemVisualEffectRecordAdaptor = GenericLegacyDBCItemVisualEffectRecordAdaptor<db_tbc::ItemVisualEffectsRecord>;
	using TBCSpellItemEnchantmentRecordAdaptor = GenericLegacyDBCSpellItemEnchantmentRecordAdaptor<db_tbc::SpellItemEnchantmentRecord>;


}