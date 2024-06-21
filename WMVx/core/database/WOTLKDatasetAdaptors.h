#pragma once
#include "GameDatasetAdaptors.h"
#include "GenericDBCDatasetAdaptors.h"
#include "ReferenceSource.h"
#include "WOTLKDefinitions.h"

namespace core {


	using WOTLKAnimationDataRecordAdaptorNext = GenericLegacyDBCAnimationDataRecordAdaptor<db_wotlk::AnimationDataRecord>;
	using WOTLKChrRacesRecordAdaptorNext = GenericLegacyDBCChrRacesRecordAdaptor <db_wotlk::ChrRacesRecord > ;
	using WOTLKCharSectionsRecordAdaptorNext = GenericLegacyDBCCharSectionsRecordAdaptor<db_wotlk::CharSectionsRecord>;

	class WOTLKCharacterFacialHairStylesRecordAdaptorNext : public CharacterFacialHairStyleRecordAdaptor, public GenericLegacyDBCRecordAdaptor<db_wotlk::CharacterFacialHairStylesRecord> {
	public:
		using GenericLegacyDBCRecordAdaptor<db_wotlk::CharacterFacialHairStylesRecord>::GenericLegacyDBCRecordAdaptor;

		constexpr uint32_t getRaceId() const override {
			return this->_record.data.raceId;
		}

		constexpr Gender getSexId() const override {
			return static_cast<Gender>(this->_record.data.sexId);
		}


		constexpr virtual uint32_t getGeoset100() const override {
			return this->_record.data.geoset[0];
		}

		constexpr virtual uint32_t getGeoset200() const override {
			return this->_record.data.geoset[2];
		}

		constexpr virtual uint32_t getGeoset300() const  override {
			return this->_record.data.geoset[1];
		}
	};

	using WOTLKCharHairGeosetsRecordAdaptorNext = GenericLegacyDBCCharacterHairGeosetsRecordAdaptor<db_wotlk::CharHairGeosetsRecord>;
	using WOTLKCreatureModelDataRecordAdaptorNext = GenericLegacyDBCCreatureModelDataRecordAdaptor<db_wotlk::CreatureModelDataRecord>;
	using WOTLKCreatureModelDisplayInfoExtraRecordAdaptorNext = GenericLegacyDBCCreatureDisplayExtraRecordAdaptor<db_wotlk::CreatureDisplayInfoExtraRecord>;
	using WOTLKCreatureDisplayInfoRecordAdaptorNext = GenericLegacyDBCCreatureDisplayRecordAdaptor<db_wotlk::CreatureDisplayInfoRecord>;

	class WOTLKItemRecordAdaptorNext : public ItemRecordAdaptor, public GenericLegacyDBCRecordAdaptor<db_wotlk::ItemRecord> {
	public:
		WOTLKItemRecordAdaptorNext(db_wotlk::ItemRecord&& record, const ReferenceSourceItemsCache::ItemCacheRecord* cache_record) :
			GenericLegacyDBCRecordAdaptor<db_wotlk::ItemRecord>(std::move(record)), cacheRecord(cache_record)
		{}
		WOTLKItemRecordAdaptorNext(WOTLKItemRecordAdaptorNext&&) = default;
		virtual ~WOTLKItemRecordAdaptorNext() = default;

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

	using WOTLKItemDisplayInfoRecordAdaptorNext = GenericLegacyDBCItemDisplayInfoRecordAdaptor<db_wotlk::ItemDisplayInfoRecord>;
	using WOTLKItemVisualRecordAdaptorNext = GenericLegacyDBCItemVisualRecordAdaptor<db_wotlk::ItemVisualsRecord>;
	using WOTLKItemVisualEffectRecordAdaptorNext = GenericLegacyDBCItemVisualEffectRecordAdaptor<db_wotlk::ItemVisualEffectsRecord>;
	using WOTLKSpellItemEnchantmentRecordAdaptorNext = GenericLegacyDBCSpellItemEnchantmentRecordAdaptor<db_wotlk::SpellItemEnchantmentRecord>;


}