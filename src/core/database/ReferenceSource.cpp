#include "../../stdafx.h"
#include "ReferenceSource.h"
#include <fast-cpp-csv-parser/csv.h>

namespace core {
	ReferenceSourceItemsCache::ReferenceSourceItemsCache(QString fileName) {

		io::CSVReader<6, io::trim_chars<>, io::double_quote_escape<',', '"'>> in(fileName.toStdString());
		in.read_header(io::ignore_no_column, "id", "name", "item_display_info_id", "item_quality_id", "inventory_slot_id", "sheath_type_id");

		uint32_t item_id;
		std::string name;
		uint32_t display_id;
		uint32_t quality_id;
		uint32_t slot_id;
		uint32_t sheath_id;

		while (in.read_row(item_id, name, display_id, quality_id, slot_id, sheath_id)) {
			ItemCacheRecord temp;
			temp.itemId = item_id;
			temp.name = QString::fromStdString(name);
			temp.itemDisplayInfoId = display_id;
			temp.itemQuality = (ItemQualityId)quality_id;
			temp.inventorySlot = (ItemInventorySlotId)slot_id;
			temp.sheatheTypeId = (SheathTypes)sheath_id;

			itemCacheRecords.push_back(std::move(temp));
		}
	}

	ReferenceSourceNPCsDataset::ReferenceSourceNPCsDataset(QString fileName) {

		io::CSVReader<4, io::trim_chars<>> in(fileName.toStdString());

		uint32_t id;
		uint32_t model_id;
		uint32_t type;
		std::string name;

		while (in.read_row(id, model_id, type, name)) {
			RSNPCRecordAdaptor::Record temp;
			temp.id = id;
			temp.modelId = model_id;
			temp.type = type;
			temp.name = QString::fromStdString(name);
			adaptors.push_back(std::make_unique<RSNPCRecordAdaptor>(temp));
		}
	}

	ReferenceSourceAnimationNames::ReferenceSourceAnimationNames(QString fileName)
	{
		io::CSVReader<2, io::trim_chars<>, io::double_quote_escape<';', '"'>> in(fileName.toStdString());
		in.read_header(io::ignore_no_column, "ID", "Name");
		std::string name; 
		uint32_t id;
		
		while (in.read_row(id, name)) {
			animationNames.insert({ id, QString::fromStdString(name) });
		}
	}
}