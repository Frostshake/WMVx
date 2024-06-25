#pragma once
#include "GameDataset.h"
#include "WOTLKDatasetAdaptors.h"
#include "../filesystem/MPQFileSystem.h"
#include "GenericDBCDataset.h"
#include <WDBReader/Database/DBCFile.hpp>

namespace core {

	using WOTLKAnimationDataDataset = GenericDBCDataset<DatasetAnimationData, WOTLKAnimationDataRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKChrRacesDataset = GenericDBCDataset<DatasetCharacterRaces, WOTLKChrRacesRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKCharacterFacialHairStylesDataset = GenericDBCDataset<DatasetCharacterFacialHairStyles, WOTLKCharacterFacialHairStylesRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKCharHairGeosetsDataset = GenericDBCDataset<DatasetCharacterHairGeosets, WOTLKCharHairGeosetsRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKCharSectionsDataset = GenericDBCDataset<DatasetCharacterSections, WOTLKCharSectionsRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKCreatureModelDataDataset = GenericDBCDataset<DatasetCreatureModelData, WOTLKCreatureModelDataRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKCreatureDisplayDataset = GenericDBCCreatureDisplayInfoDataset<WOTLKCreatureDisplayInfoRecordAdaptor, WOTLKCreatureModelDisplayInfoExtraRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;

	class WOTLKItemDataset : public DatasetItems, protected ReferenceSourceItemsCache {
	public:
		using Adaptor = WOTLKItemRecordAdaptor;

		WOTLKItemDataset(MPQFileSystem* fs, QString itemReferenceFileName) :
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
		WOTLKItemDataset(WOTLKItemDataset&&) = default;
		virtual ~WOTLKItemDataset() = default;

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::vector<std::unique_ptr<Adaptor>> adaptors;
	};

	using WOTLKItemDisplayInfoDataset = GenericDBCDataset<DatasetItemDisplay, WOTLKItemDisplayInfoRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKItemVisualDataset = GenericDBCDataset<DatasetItemVisual, WOTLKItemVisualRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKItemVisualEffectDataset = GenericDBCDataset<DatasetItemVisualEffect, WOTLKItemVisualEffectRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using WOTLKSpellItemEnchantmentDataset = GenericDBCDataset<DatasetSpellItemEnchantment, WOTLKSpellItemEnchantmentRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;

};