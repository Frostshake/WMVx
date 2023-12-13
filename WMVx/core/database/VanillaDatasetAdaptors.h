#pragma once

#include "GameDatasetAdaptors.h"
#include "VanillaRecordDefinitions.h"
#include "DBCFile.h"
#include "DBCBackedDataset.h"
#include "GenericDBCDatasetAdaptors.h"
#include "ReferenceSource.h"

namespace core {

	using VanillaAnimationDataRecordAdaptor = GenericDBCAnimationDataRecordAdaptor<VanillaDBCAnimationDataRecord>;

	using VanillaCharRacesRecordAdaptor = GenericDBCCharacterRacesRecordAdaptor<VanillaDBCCharRacesRecord>;

	class VanillaCharSectionsRecordAdaptor : public CharacterSectionRecordAdaptor, public DBCBackedAdaptor<VanillaDBCCharSectionsRecord> {
	public:
		using DBCBackedAdaptor<VanillaDBCCharSectionsRecord>::DBCBackedAdaptor;

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

	class VanillaCharacterFacialHairStylesRecordAdaptor : public CharacterFacialHairStyleRecordAdaptor, public DBCBackedAdaptor<VanillaDBCCharacterFacialHairStylesRecord> {
	public:
		using DBCBackedAdaptor<VanillaDBCCharacterFacialHairStylesRecord>::DBCBackedAdaptor;

		constexpr uint32_t getRaceId() const override  {
			return this->handle->raceId;
		}

		constexpr Gender getSexId() const override  {
			return static_cast<Gender>(this->handle->sexId);
		}

		constexpr uint32_t getGeoset100() const override {
			return this->handle->geoset4;
		}

		constexpr uint32_t getGeoset200() const override {
			return this->handle->geoset5;
		}

		constexpr uint32_t getGeoset300() const override {
			return this->handle->geoset6;
		}
	};


	using VanillaCharHairGeosetsRecordAdaptor = GenericDBCCharacterHairGeosetsRecordAdaptor<VanillaDBCCharHairGeosetsRecord>;

	using VanillaCreatureModelDataRecordAdaptor = GenericDBCCreatureModelDataRecordAdaptor<VanillaDBCCreatureModelDataRecord>;

	using VanillaCreatureModelDisplayInfoExtraRecordAdaptor = GenericDBCCreatureDisplayExtraRecordAdaptor<VanillaDBCCreatureDisplayInfoExtraRecord>;

	class VanillaCreatureDisplayInfoRecordAdaptor : public CreatureDisplayRecordAdaptor, public DBCBackedAdaptor<VanillaDBCCreatureDisplayInfoRecord> {
	public:
		using DBCBackedAdaptor<VanillaDBCCreatureDisplayInfoRecord>::DBCBackedAdaptor;

		VanillaCreatureDisplayInfoRecordAdaptor(const VanillaDBCCreatureDisplayInfoRecord* handle,
			const DBCFile<VanillaDBCCreatureDisplayInfoRecord>* dbc,
			const VanillaDBCCreatureDisplayInfoExtraRecord* extra)
			: DBCBackedAdaptor<VanillaDBCCreatureDisplayInfoRecord>(handle, dbc) {
			if (extra != nullptr) {
				extra_adaptor = std::make_unique<VanillaCreatureModelDisplayInfoExtraRecordAdaptor>(extra, nullptr);
			}
		}
		VanillaCreatureDisplayInfoRecordAdaptor(VanillaCreatureDisplayInfoRecordAdaptor&&) = default;
		virtual ~VanillaCreatureDisplayInfoRecordAdaptor() {}

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
		std::unique_ptr<VanillaCreatureModelDisplayInfoExtraRecordAdaptor> extra_adaptor;
	};

	class VanillaItemRecordAdaptor : public ItemRecordAdaptor {
	public:
		VanillaItemRecordAdaptor(const ReferenceSourceItemsCache::ItemCacheRecord* cache_record) 
			: cacheRecord(cache_record) {}
		VanillaItemRecordAdaptor(VanillaItemRecordAdaptor&&) = default;
		virtual ~VanillaItemRecordAdaptor() {}

		constexpr uint32_t getId() const override {
			return cacheRecord->itemId;
		}

		constexpr uint32_t getItemDisplayInfoId() const override {
			return cacheRecord->itemDisplayInfoId;
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

	using VanillaItemDisplayInfoRecordAdaptor = GenericDBCItemDisplayInfoRecordAdaptor<VanillaDBCItemDisplayInfoRecord>;

	using VanillaItemVisualRecordAdaptor = GenericDBCItemVisualRecordAdaptor<VanillaDBCItemVisualRecord>;

	using VanillaItemVisualEffectRecordAdaptor = GenericDBCItemVisualEffectRecordAdaptor<VanillaDBCItemVisualEffectRecord>;

	using VanillaSpellItemEnchantmentRecordAdaptor = GenericDBCSpellItemEnchantmentRecordAdaptor<VanillaDBCSpellItemEnchantmentRecord>;
}