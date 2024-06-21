#pragma once
#include "GameDataset.h"
#include "WOTLKDatasetAdaptors.h"
#include "../filesystem/MPQFileSystem.h"
#include "GenericDBCDataset.h"
#include <WDBReader/Database/DBCFile.hpp>

namespace core {

	using WOTLKAnimationDataDatasetNext = GenericLegacyDBCDataset<DatasetAnimationData, WOTLKAnimationDataRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKChrRacesDatasetNext = GenericLegacyDBCDataset<DatasetCharacterRaces, WOTLKChrRacesRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKCharacterFacialHairStylesDatasetNext = GenericLegacyDBCDataset<DatasetCharacterFacialHairStyles, WOTLKCharacterFacialHairStylesRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKCharHairGeosetsDatasetNext = GenericLegacyDBCDataset<DatasetCharacterHairGeosets, WOTLKCharHairGeosetsRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKCharSectionsDatasetNext = GenericLegacyDBCDataset<DatasetCharacterSections, WOTLKCharSectionsRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKCreatureModelDataDatasetNext = GenericLegacyDBCDataset<DatasetCreatureModelData, WOTLKCreatureModelDataRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKCreatureDisplayDatasetNext = GenericLegacyDBCCreatureDisplayInfoDataset<DatasetCreatureDisplay, WOTLKCreatureDisplayInfoRecordAdaptorNext, WOTLKCreatureModelDisplayInfoExtraRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;

	class WOTLKItemDatasetNext : public DatasetItems, protected ReferenceSourceItemsCache {
	public:
		using Adaptor = WOTLKItemRecordAdaptorNext;

		WOTLKItemDatasetNext(MPQFileSystem* fs, QString itemReferenceFileName) :
			DatasetItems(),
			ReferenceSourceItemsCache(itemReferenceFileName) {

			auto file = fs->openFile("DBFilesClient\\Item.dbc");
			if (file == nullptr) {
				throw std::runtime_error("Unable to open dbc.");
			}

			auto dbc = WDBReader::Database::makeDBCFile<Adaptor::Record, WDBReader::Filesystem::MPQFileSource>(WDBReader::Database::DBCVersion::BC_WOTLK);

			dbc.open(static_cast<MPQFile*>(file.get())->release());
			dbc.load();
			adaptors.reserve(dbc.size());

			for (auto& rec : dbc) {

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
		WOTLKItemDatasetNext(WOTLKItemDatasetNext&&) = default;
		virtual ~WOTLKItemDatasetNext() = default;

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::vector<std::unique_ptr<Adaptor>> adaptors;
	};

	using WOTLKItemDisplayInfoDatasetNext = GenericLegacyDBCDataset<DatasetItemDisplay, WOTLKItemDisplayInfoRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKItemVisualDatasetNext = GenericLegacyDBCDataset<DatasetItemVisual, WOTLKItemVisualRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKItemVisualEffectDatasetNext = GenericLegacyDBCDataset<DatasetItemVisualEffect, WOTLKItemVisualEffectRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKSpellItemEnchantmentDatasetNext = GenericLegacyDBCDataset<DatasetSpellItemEnchantment, WOTLKSpellItemEnchantmentRecordAdaptorNext, WDBReader::Database::DBCVersion::BC_WOTLK>;

};