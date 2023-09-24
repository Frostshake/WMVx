#include "../../stdafx.h"
#include "ReferenceSource.h"

namespace core {
	ReferenceSourceItemsCache::ReferenceSourceItemsCache(QString fileName) {
		headerFound = false;
		QFile source(fileName);
		if (source.open(QIODevice::ReadOnly)) {
			QtCSV::Reader::readToProcessor(source, *this);
			source.close();
		}
	}

	bool ReferenceSourceItemsCache::processRowElements(const QList<QString>& elements)
	{
		if (!headerFound) {
			headerFound = true;
			return elements.length() == 6;
		}

		if (elements.length() == 6) {
			ItemCacheRecord temp;
			temp.itemId = elements[0].toUInt();
			temp.name = elements[1];
			temp.itemDisplayInfoId = elements[2].toUInt();
			temp.itemQuality = (ItemQualityId)elements[3].toUInt();
			temp.inventorySlot = (ItemInventorySlotId)elements[4].toUInt();
			temp.sheatheTypeId = (SheathTypes)elements[5].toUInt();

			itemCacheRecords.push_back(temp);
		}

		return true;
	}

	ReferenceSourceNPCsDataset::ReferenceSourceNPCsDataset(QString fileName) {
		QFile source(fileName);
		if (source.open(QIODevice::ReadOnly)) {
			QTextStream in(&source);
			while (!in.atEnd())
			{
				QStringList parts = in.readLine().split(',');

				//TODO improve logic, use real csv reader.
				if (parts.length() == 4) {
					RSNPCRecordAdaptor::Record temp;
					temp.id = parts[0].toUInt();
					temp.modelId = parts[1].toUInt();
					temp.type = parts[2].toUInt();
					temp.name = parts[3];
					adaptors.push_back(std::make_unique<RSNPCRecordAdaptor>(temp));
				}
			}

			source.close();
		}
	}



	ReferenceSourceAnimationNames::ReferenceSourceAnimationNames(QString fileName)
	{
		headerFound = false;
		QFile source(fileName);
		if (source.open(QIODevice::ReadOnly)) {
			QtCSV::Reader::readToProcessor(source, *this, ";");
			source.close();
		}
	}

	bool ReferenceSourceAnimationNames::processRowElements(const QList<QString>& elements)
	{
		if (!headerFound) {
			headerFound = true;
			return elements.length() == 2;
		}

		if (elements.length() == 2) {
			animationNames.insert({ elements[0].toUInt(), elements[1] });
		}

		return true;
	}

}