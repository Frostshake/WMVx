#pragma once
#include "GameDataset.h"
#include "CataDatasetAdaptors.h"
#include "../filesystem/MPQFileSystem.h"
#include "GenericDBCDataset.h"
#include <WDBReader/Database/DBCFile.hpp>
#include <WDBReader/Database/DB2File.hpp>

namespace core {

	using CataAnimationDataDataset = GenericDBCDataset<DatasetAnimationData, CataAnimationDataRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;
	using CataChrRacesDataset = GenericDBCDataset<DatasetCharacterRaces, CataChrRacesRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;
	using CataCharacterFacialHairStylesDataset = GenericDBCDataset<DatasetCharacterFacialHairStyles, CataCharacterFacialHairStylesRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;
	using CataCharHairGeosetsDataset = GenericDBCDataset<DatasetCharacterHairGeosets, CataCharHairGeosetsRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;
	using CataCharSectionsDataset = GenericDBCDataset<DatasetCharacterSections, CataCharSectionsRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;
	using CataCreatureModelDataDataset = GenericDBCDataset<DatasetCreatureModelData, CataCreatureModelDataRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;
	using CataCreatureDisplayDataset = GenericDBCCreatureDisplayInfoDataset<CataCreatureDisplayInfoRecordAdaptor, CataCreatureModelDisplayInfoExtraRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;

	class CataItemDataset : public DatasetItems, protected ReferenceSourceItemsCache {
	public:
		using Adaptor = CataItemRecordAdaptor;

		CataItemDataset(MPQFileSystem* fs, QString itemReferenceFileName) :
			DatasetItems(),
			ReferenceSourceItemsCache(itemReferenceFileName) {

			auto file = fs->openFile("DBFilesClient\\Item.db2");
			if (file == nullptr) {
				throw std::runtime_error("Unable to open db2.");
			}
			
			auto db2 = WDBReader::Database::makeDB2FileFormat<decltype(Adaptor::Record::schema), Adaptor::Record, WDBReader::Filesystem::FileSource, WDBReader::Database::DB2FileFormatWDB2>(
				Adaptor::Record::schema, file->release()
			);

			adaptors.reserve(db2->size());

			for (auto& rec : *db2) {

				ReferenceSourceItemsCache::ItemCacheRecord* cacheptr = nullptr;
				auto temp = std::find_if(itemCacheRecords.begin(), itemCacheRecords.end(), [&rec](const ReferenceSourceItemsCache::ItemCacheRecord& cache_record) -> bool {
					return rec.data.id == cache_record.itemId;
					});

				if (temp != itemCacheRecords.end()) {
					cacheptr = &(*temp);
				}

				adaptors.push_back(
					std::make_unique<Adaptor>(std::move(rec), cacheptr)
				);
			}

		}
		CataItemDataset(CataItemDataset&&) = default;
		virtual ~CataItemDataset() = default;

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::vector<std::unique_ptr<Adaptor>> adaptors;
	};

	using CataItemDisplayInfoDataset = GenericDBCDataset<DatasetItemDisplay, CataItemDisplayInfoRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;
	using CataItemVisualDataset = GenericDBCDataset<DatasetItemVisual, CataItemVisualRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;
	using CataItemVisualEffectDataset = GenericDBCDataset<DatasetItemVisualEffect, CataItemVisualEffectRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;
	using CataSpellItemEnchantmentDataset = GenericDBCDataset<DatasetSpellItemEnchantment, CataSpellItemEnchantmentRecordAdaptor, WDBReader::Database::DBCVersion::CATA_PLUS>;

};