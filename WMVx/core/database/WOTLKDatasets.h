#pragma once
#include "GameDataset.h"
#include "WOTLKDatasetAdaptors.h"
#include "../filesystem/MPQFileSystem.h"
#include "DBCFile.h"
#include "DBCBackedDataset.h"

#define BOOST_METAPARSE_LIMIT_STRING_SIZE 64

#include "boost/mpl/string.hpp"
#include "boost/metaparse/string.hpp"
#include "GenericDBCDataset.h"

namespace core {


	using WOTLKAnimationDataDataset = GenericDBCDataset<DatasetAnimationData, WOTLKAnimationDataRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\AnimationData.dbc")>::value>;

	using WOTLKCharRacesDataset = GenericDBCDataset<DatasetCharacterRaces, WOTLKCharRacesRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\ChrRaces.dbc")>::value>;

	using WOTLKCharacterFacialHairStylesDataset = GenericDBCDataset<DatasetCharacterFacialHairStyles, WOTLKCharacterFacialHairStylesRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\CharacterFacialHairStyles.dbc")>::value>;

	using WOTLKCharHairGeosetsDataset = GenericDBCDataset<DatasetCharacterHairGeosets, WOTLKCharHairGeosetsRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\CharHairGeosets.dbc")>::value>;

	using WOTLKCharSectionsDataset = GenericDBCDataset<DatasetCharacterSections, WOTLKCharSectionsRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\CharSections.dbc")>::value>;

	using WOTLKCreatureModelDataDataset = GenericDBCDataset<DatasetCreatureModelData, WOTLKCreatureModelDataRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\CreatureModelData.dbc")>::value>;

	class WOTLKCreatureDisplayDataset : public DatasetCreatureDisplay, public DBCBackedDataset<WOTLKCreatureDisplayInfoRecordAdaptor, CreatureDisplayRecordAdaptor, false> {
		public:
			using Adaptor = WOTLKCreatureDisplayInfoRecordAdaptor;
			WOTLKCreatureDisplayDataset(MPQFileSystem* fs) : 
				DatasetCreatureDisplay(),
				DBCBackedDataset<WOTLKCreatureDisplayInfoRecordAdaptor, CreatureDisplayRecordAdaptor, false>(fs, "DBFilesClient\\CreatureDisplayInfo.dbc") {

				dbc_extra = std::make_unique<DBCFile<WOTLKDBCCreatureDisplayInfoExtraRecord>>("DBFilesClient\\CreatureDisplayInfoExtra.dbc");
				dbc_extra->open(fs);

				auto& extra_records = dbc_extra->getRecords();
				auto& records = dbc->getRecords();
				for (auto it = records.begin(); it != records.end(); ++it) {	

					WOTLKDBCCreatureDisplayInfoExtraRecord* extra = nullptr;

					if (it->extraDisplayInformationId > 0) {
						for (auto it2 = extra_records.begin(); it2 != extra_records.end(); ++it2) {
							if (it2->id == it->extraDisplayInformationId) {
								extra = &(*it2);
								break;
							}
						}
					}

					adaptors.push_back(
						std::make_unique<Adaptor>(&(*it), dbc.get(), extra)
					);
				}
			}
			WOTLKCreatureDisplayDataset(WOTLKCreatureDisplayDataset&&) = default;
			virtual ~WOTLKCreatureDisplayDataset() {}

			const std::vector<CreatureDisplayRecordAdaptor*>& all() const override {
				return reinterpret_cast<const std::vector<CreatureDisplayRecordAdaptor*>&>(this->adaptors);
			}

		protected:
			std::unique_ptr<DBCFile<WOTLKDBCCreatureDisplayInfoExtraRecord>> dbc_extra;
	};

	using WOTLKItemDisplayInfoDataset = GenericDBCDataset<DatasetItemDisplay, WOTLKItemDisplayInfoRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\ItemDisplayInfo.dbc")>::value>;

	using WOTLKItemVisualDataset = GenericDBCDataset<DatasetItemVisual, WOTLKItemVisualRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\ItemVisuals.dbc")>::value>;

	using WOTLKItemVisualEffectDataset = GenericDBCDataset<DatasetItemVisualEffect, WOTLKItemVisualEffectRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\ItemVisualEffects.dbc")>::value>;

	using WOTLKSpellItemEnchantmentDataset = GenericDBCDataset<DatasetSpellItemEnchantment, WOTLKSpellItemEnchantmentRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\SpellItemEnchantment.dbc")>::value>;

	class WOTLKItemDataset : public DatasetItems, protected ReferenceSourceItemsCache {
	public:
		using Adaptor = WOTLKItemRecordAdaptor;

		WOTLKItemDataset(MPQFileSystem* fs, QString itemReferenceFileName) : 
			DatasetItems(),
			ReferenceSourceItemsCache(itemReferenceFileName) {
	

			dbc = std::make_unique<DBCFile<WOTLKDBCItemRecord>>("DBFilesClient\\Item.dbc");
			dbc->open(fs);

			auto& records = dbc->getRecords();
			for (auto it = records.begin(); it != records.end(); ++it) {

				ReferenceSourceItemsCache::ItemCacheRecord* cacheptr = nullptr;
				auto temp = std::find_if(itemCacheRecords.begin(), itemCacheRecords.end(), [it](const ReferenceSourceItemsCache::ItemCacheRecord& cache_record) -> bool {
					return it->id == cache_record.itemId;
				});

				if (temp != itemCacheRecords.end()) {
					cacheptr = &(*temp);
				}

				adaptors.push_back(
					std::make_unique<Adaptor>(&(*it), dbc.get(), cacheptr)
				);
			}
		}
		WOTLKItemDataset(WOTLKItemDataset&&) = default;
		virtual ~WOTLKItemDataset() {}

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->adaptors);
		}

	protected:

		std::unique_ptr<DBCFile<typename Adaptor::Record>> dbc;
		std::vector<std::unique_ptr<Adaptor>> adaptors;

	};
};