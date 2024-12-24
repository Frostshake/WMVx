#pragma once
#include "GameDatasetAdaptors.h"
#include "GenericDBCDatasetAdaptors.h"
#include "ReferenceSource.h"
#include "CataDefinitions.h"

namespace core {


	using CataAnimationDataRecordAdaptor = GenericLegacyDBCAnimationDataRecordAdaptor<db_cata::AnimationDataRecord>;
	using CataChrRacesRecordAdaptor = GenericLegacyDBCChrRacesRecordAdaptor <db_cata::ChrRacesRecord >;
	using CataCharSectionsRecordAdaptor = GenericLegacyDBCCharSectionsRecordAdaptor<db_cata::CharSectionsRecord>;

	class CataCharacterFacialHairStylesRecordAdaptor : public CharacterFacialHairStyleRecordAdaptor, public GenericDBCRecordAdaptor<db_cata::CharacterFacialHairStylesRecord> {
	public:
		using GenericDBCRecordAdaptor<db_cata::CharacterFacialHairStylesRecord>::GenericDBCRecordAdaptor;

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

	using CataCharHairGeosetsRecordAdaptor = GenericLegacyDBCCharacterHairGeosetsRecordAdaptor<db_cata::CharHairGeosetsRecord>;
	using CataCreatureModelDataRecordAdaptor = GenericLegacyDBCCreatureModelDataRecordAdaptor<db_cata::CreatureModelDataRecord>;
	using CataCreatureModelDisplayInfoExtraRecordAdaptor = GenericLegacyDBCCreatureDisplayExtraRecordAdaptor<db_cata::CreatureDisplayInfoExtraRecord>;
	using CataCreatureDisplayInfoRecordAdaptor = GenericLegacyDBCCreatureDisplayRecordAdaptor<db_cata::CreatureDisplayInfoRecord>;

	class CataItemRecordAdaptor : public ItemRecordAdaptor, public GenericDBCRecordAdaptor<db_cata::ItemRecord> {
	public:
		CataItemRecordAdaptor(db_cata::ItemRecord&& record, const ReferenceSourceItemsCache::ItemCacheRecord* cache_record) :
			GenericDBCRecordAdaptor<db_cata::ItemRecord>(std::move(record)), cacheRecord(cache_record)
		{}
		CataItemRecordAdaptor(CataItemRecordAdaptor&&) = default;
		virtual ~CataItemRecordAdaptor() = default;

		constexpr uint32_t getId() const override {
			return this->_record.data.id;
		}

		constexpr std::vector<uint32_t> getItemDisplayInfoId() const override {
			return { this->_record.data.displayInfoId };
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

	using CataItemDisplayInfoRecordAdaptor = GenericLegacyDBCItemDisplayInfoRecordAdaptor<db_cata::ItemDisplayInfoRecord>;
	using CataItemVisualRecordAdaptor = GenericLegacyDBCItemVisualRecordAdaptor<db_cata::ItemVisualsRecord>;
	using CataItemVisualEffectRecordAdaptor = GenericLegacyDBCItemVisualEffectRecordAdaptor<db_cata::ItemVisualEffectsRecord>;
	using CataSpellItemEnchantmentRecordAdaptor = GenericLegacyDBCSpellItemEnchantmentRecordAdaptor<db_cata::SpellItemEnchantmentRecord>;


}