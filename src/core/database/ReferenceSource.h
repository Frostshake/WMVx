#pragma once

#include <vector>
#include <QString>
#include "../game/GameConstants.h"
#include "GameDataset.h"

namespace core {

	// not all data can be found in client files (e.g items)
	// this is used as a base class for loading included wmvx reference data.

	class ReferenceSourceItemsCache {
	public:
		struct ItemCacheRecord {
			uint32_t itemId;
			QString name;
			uint32_t itemDisplayInfoId;
			ItemQualityId itemQuality;
			ItemInventorySlotId inventorySlot;
			SheathTypes sheatheTypeId;
		};

		ReferenceSourceItemsCache(QString fileName);
		ReferenceSourceItemsCache(ReferenceSourceItemsCache&&) = default;
		virtual ~ReferenceSourceItemsCache() {}

		const std::vector<ItemCacheRecord>& records() const {
			return itemCacheRecords;
		}

	protected:
		std::vector<ItemCacheRecord> itemCacheRecords;

			
	private:
		bool headerFound;
	};

	class ReferenceSourceAnimationNames {
	public:
		ReferenceSourceAnimationNames(QString fileName);
		ReferenceSourceAnimationNames(ReferenceSourceAnimationNames&&) = default;
		virtual ~ReferenceSourceAnimationNames() {}

		const std::map<uint32_t, QString>& records() const {
			return animationNames;
		}

	protected:
		std::map<uint32_t, QString> animationNames;

	private:
		bool headerFound;
	};


	class RSNPCRecordAdaptor : public NPCRecordAdaptor {
	public:
		struct Record {
			uint32_t id;
			uint32_t modelId;
			uint32_t type;
			QString name;
		};

		RSNPCRecordAdaptor(Record record) {
			this->record = record;
		}

		constexpr uint32_t getId() const override {
			return record.id;
		}

		constexpr uint32_t getModelId() const override {
			return record.modelId;
		}

		constexpr uint32_t getType() const override {
			return record.type;
		}

		QString getName() const override {
			return record.name;
		}

	protected:
		Record record;
	};

	class ReferenceSourceNPCsDataset : public DatasetNPCs {
	public:

		ReferenceSourceNPCsDataset(QString fileName);
		ReferenceSourceNPCsDataset(ReferenceSourceNPCsDataset&&) = default;
		virtual ~ReferenceSourceNPCsDataset() {}

		const std::vector<NPCRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<NPCRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::vector<std::unique_ptr<RSNPCRecordAdaptor>> adaptors;
	};

};