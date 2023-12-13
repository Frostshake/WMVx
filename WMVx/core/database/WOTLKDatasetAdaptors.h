#pragma once
#include "GameDatasetAdaptors.h"
#include "WOTLKRecordDefinitions.h"
#include "DBCBackedDataset.h"
#include "GenericDBCDatasetAdaptors.h"
#include "ReferenceSource.h"

namespace core {


	using WOTLKAnimationDataRecordAdaptor = GenericDBCAnimationDataRecordAdaptor<WOTLKDBCAnimationDataRecord>;

	using WOTLKCharRacesRecordAdaptor = GenericDBCCharacterRacesRecordAdaptor<WOTLKDBCCharRacesRecord>;

	using WOTLKCharacterFacialHairStylesRecordAdaptor = GenericDBCCharacterFacialHairStylesRecordAdaptor<WOTLKDBCCharacterFacialHairStylesRecord>;

	using WOTLKCharHairGeosetsRecordAdaptor = GenericDBCCharacterHairGeosetsRecordAdaptor<WOTLKDBCCharHairGeosetsRecord>;

	class WOTLKCharSectionsRecordAdaptor : public CharacterSectionRecordAdaptor, public DBCBackedAdaptor<WOTLKDBCCharSectionsRecord> {
	public:
		using DBCBackedAdaptor<WOTLKDBCCharSectionsRecord>::DBCBackedAdaptor;

		constexpr uint32_t getId() const override {
			return handle->id;
		}

		constexpr uint32_t getRaceId() const override {
			return handle->raceId;
		}

		constexpr Gender getSexId() const override {
			return static_cast<Gender>(handle->sexId);
		}

		constexpr CharacterSectionType getType() const override {
			return static_cast<CharacterSectionType>(handle->type);
		}

		std::array<GameFileUri, 3> getTextures() const override {
			return {
				dbc->getString(handle->texture1),
				dbc->getString(handle->texture2),
				dbc->getString(handle->texture3)
			};
		}

		constexpr uint32_t getSection() const override {
			return handle->section;
		}

		constexpr uint32_t getVariationIndex() const override {
			return handle->variationIndex;
		}

		constexpr bool isHD() const override {
			return false;
		}
	};

	using WOTLKCreatureModelDataRecordAdaptor = GenericDBCCreatureModelDataRecordAdaptor<WOTLKDBCCreatureModelDataRecord>;

	using WOTLKCreatureModelDisplayInfoExtraRecordAdaptor = GenericDBCCreatureDisplayExtraRecordAdaptor<WOTLKDBCCreatureDisplayInfoExtraRecord>;

	class WOTLKCreatureDisplayInfoRecordAdaptor : public CreatureDisplayRecordAdaptor, public DBCBackedAdaptor<WOTLKDBCCreatureDisplayInfoRecord> {
	public:
		using DBCBackedAdaptor<WOTLKDBCCreatureDisplayInfoRecord>::DBCBackedAdaptor;

		WOTLKCreatureDisplayInfoRecordAdaptor(const WOTLKDBCCreatureDisplayInfoRecord* handle,
			const DBCFile<WOTLKDBCCreatureDisplayInfoRecord>* dbc,
			const WOTLKDBCCreatureDisplayInfoExtraRecord* extra)
			: DBCBackedAdaptor<WOTLKDBCCreatureDisplayInfoRecord>(handle, dbc) {
			if (extra != nullptr) {
				extra_adaptor = std::make_unique<WOTLKCreatureModelDisplayInfoExtraRecordAdaptor>(extra, nullptr);
			}
		}
		WOTLKCreatureDisplayInfoRecordAdaptor(WOTLKCreatureDisplayInfoRecordAdaptor&&) = default;
		virtual ~WOTLKCreatureDisplayInfoRecordAdaptor() {}

		constexpr uint32_t getId() const override {
			return handle->id;
		}

		constexpr uint32_t getModelId() const override {
			return handle->modelId;
		}

		std::array<GameFileUri, 3> getTextures() const override {
			return {
				dbc->getString(handle->texture[0]),
				dbc->getString(handle->texture[1]),
				dbc->getString(handle->texture[2])
			};
		}

		const CreatureDisplayExtraRecordAdaptor* getExtra() const override {
			return reinterpret_cast<const CreatureDisplayExtraRecordAdaptor*>(extra_adaptor.get());
		}

	protected:
		std::unique_ptr<WOTLKCreatureModelDisplayInfoExtraRecordAdaptor> extra_adaptor;
	};

	class WOTLKItemRecordAdaptor : public ItemRecordAdaptor, public DBCBackedAdaptor<WOTLKDBCItemRecord> {
	public:
		WOTLKItemRecordAdaptor(const WOTLKDBCItemRecord* handle, 
			const DBCFile<WOTLKDBCItemRecord>* dbc, 
			const ReferenceSourceItemsCache::ItemCacheRecord* cache_record)
			: DBCBackedAdaptor(handle, dbc), cacheRecord(cache_record) {}
		WOTLKItemRecordAdaptor(WOTLKItemRecordAdaptor&&) = default;
		virtual ~WOTLKItemRecordAdaptor() {}

		constexpr uint32_t getId() const override {
			return handle->id;
		}

		constexpr uint32_t getItemDisplayInfoId() const override {
			return handle->itemDisplayInfoId;
		}

		constexpr ItemInventorySlotId getInventorySlotId() const override {
			return (ItemInventorySlotId)handle->inventorySlotId;
		}

		constexpr SheathTypes getSheatheTypeId() const override {
			return (SheathTypes)handle->sheatheTypeId;
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

	using WOTLKItemDisplayInfoRecordAdaptor = GenericDBCItemDisplayInfoRecordAdaptor<WOTLKDBCItemDisplayInfoRecord>;

	using WOTLKItemVisualRecordAdaptor = GenericDBCItemVisualRecordAdaptor<WOTLKDBCItemVisualRecord>;

	using WOTLKItemVisualEffectRecordAdaptor = GenericDBCItemVisualEffectRecordAdaptor<WOTLKDBCItemVisualEffectRecord>;

	using WOTLKSpellItemEnchantmentRecordAdaptor = GenericDBCSpellItemEnchantmentRecordAdaptor<WOTLKDBCSpellItemEnchantmentRecord>;

}