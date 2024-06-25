#pragma once
#include "GameDatasetAdaptors.h"
#include "GenericDBCDatasetAdaptors.h"
#include "ReferenceSource.h"
#include "WOTLKDefinitions.h"

namespace core {


	using WOTLKAnimationDataRecordAdaptor = GenericLegacyDBCAnimationDataRecordAdaptor<db_wotlk::AnimationDataRecord>;
	using WOTLKChrRacesRecordAdaptor = GenericLegacyDBCChrRacesRecordAdaptor <db_wotlk::ChrRacesRecord > ;
	using WOTLKCharSectionsRecordAdaptor = GenericLegacyDBCCharSectionsRecordAdaptor<db_wotlk::CharSectionsRecord>;

	class WOTLKCharacterFacialHairStylesRecordAdaptor : public CharacterFacialHairStyleRecordAdaptor, public GenericDBCRecordAdaptor<db_wotlk::CharacterFacialHairStylesRecord> {
	public:
		using GenericDBCRecordAdaptor<db_wotlk::CharacterFacialHairStylesRecord>::GenericDBCRecordAdaptor;

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

	using WOTLKCharHairGeosetsRecordAdaptor = GenericLegacyDBCCharacterHairGeosetsRecordAdaptor<db_wotlk::CharHairGeosetsRecord>;
	using WOTLKCreatureModelDataRecordAdaptor = GenericLegacyDBCCreatureModelDataRecordAdaptor<db_wotlk::CreatureModelDataRecord>;
	using WOTLKCreatureModelDisplayInfoExtraRecordAdaptor = GenericLegacyDBCCreatureDisplayExtraRecordAdaptor<db_wotlk::CreatureDisplayInfoExtraRecord>;
	using WOTLKCreatureDisplayInfoRecordAdaptor = GenericLegacyDBCCreatureDisplayRecordAdaptor<db_wotlk::CreatureDisplayInfoRecord>;

	class WOTLKItemRecordAdaptor : public ItemRecordAdaptor, public GenericDBCRecordAdaptor<db_wotlk::ItemRecord> {
	public:
		WOTLKItemRecordAdaptor(db_wotlk::ItemRecord&& record, const ReferenceSourceItemsCache::ItemCacheRecord* cache_record) :
			GenericDBCRecordAdaptor<db_wotlk::ItemRecord>(std::move(record)), cacheRecord(cache_record)
		{}
		WOTLKItemRecordAdaptor(WOTLKItemRecordAdaptor&&) = default;
		virtual ~WOTLKItemRecordAdaptor() = default;

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

	using WOTLKItemDisplayInfoRecordAdaptor = GenericLegacyDBCItemDisplayInfoRecordAdaptor<db_wotlk::ItemDisplayInfoRecord>;
	using WOTLKItemVisualRecordAdaptor = GenericLegacyDBCItemVisualRecordAdaptor<db_wotlk::ItemVisualsRecord>;
	using WOTLKItemVisualEffectRecordAdaptor = GenericLegacyDBCItemVisualEffectRecordAdaptor<db_wotlk::ItemVisualEffectsRecord>;
	using WOTLKSpellItemEnchantmentRecordAdaptor = GenericLegacyDBCSpellItemEnchantmentRecordAdaptor<db_wotlk::SpellItemEnchantmentRecord>;


}