#pragma once
#include "GameDataset.h"
#include "TBCDatasetAdaptors.h"
#include "../filesystem/MPQFileSystem.h"
#include "GenericDBCDataset.h"
#include <WDBReader/Database/DBCFile.hpp>

namespace core {

	using TBCAnimationDataDataset = GenericDBCDataset<DatasetAnimationData, TBCAnimationDataRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using TBCChrRacesDataset = GenericDBCDataset<DatasetCharacterRaces, TBCChrRacesRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using TBCCharacterFacialHairStylesDataset = GenericDBCDataset<DatasetCharacterFacialHairStyles, TBCCharacterFacialHairStylesRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using TBCCharHairGeosetsDataset = GenericDBCDataset<DatasetCharacterHairGeosets, TBCCharHairGeosetsRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using TBCCharSectionsDataset = GenericDBCDataset<DatasetCharacterSections, TBCCharSectionsRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using TBCCreatureModelDataDataset = GenericDBCDataset<DatasetCreatureModelData, TBCCreatureModelDataRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using TBCCreatureDisplayDataset = GenericDBCCreatureDisplayInfoDataset<TBCCreatureDisplayInfoRecordAdaptor, TBCCreatureModelDisplayInfoExtraRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;

	class TBCItemDataset : public DatasetItems, protected ReferenceSourceItemsCache {
	public:
		using Adaptor = TBCItemRecordAdaptor;

		TBCItemDataset(MPQFileSystem* fs, QString itemReferenceFileName) :
			DatasetItems(),
			ReferenceSourceItemsCache(itemReferenceFileName) {

			auto file = fs->openFile("DBFilesClient\\Item.dbc");
			if (file == nullptr) {
				throw std::runtime_error("Unable to open dbc.");
			}

			auto dbc = WDBReader::Database::makeDBCFile<Adaptor::Record, WDBReader::Filesystem::FileSource>(WDBReader::Database::DBCVersion::BC_WOTLK);

			dbc.open(file->release());
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
		TBCItemDataset(TBCItemDataset&&) = default;
		virtual ~TBCItemDataset() = default;

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::vector<std::unique_ptr<Adaptor>> adaptors;
	};

	using TBCItemDisplayInfoDataset = GenericDBCDataset<DatasetItemDisplay, TBCItemDisplayInfoRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using TBCItemVisualDataset = GenericDBCDataset<DatasetItemVisual, TBCItemVisualRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using TBCItemVisualEffectDataset = GenericDBCDataset<DatasetItemVisualEffect, TBCItemVisualEffectRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;
	using TBCSpellItemEnchantmentDataset = GenericDBCDataset<DatasetSpellItemEnchantment, TBCSpellItemEnchantmentRecordAdaptor, WDBReader::Database::DBCVersion::BC_WOTLK>;

};