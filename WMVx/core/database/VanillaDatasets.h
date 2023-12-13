#pragma once

#include "GameDataset.h"
#include "VanillaDatasetAdaptors.h"
#include "../filesystem/MPQFileSystem.h"
#include "DBCFile.h"
#include "DBCBackedDataset.h"

#define BOOST_METAPARSE_LIMIT_STRING_SIZE 64

#include "boost/mpl/string.hpp"
#include "boost/metaparse/string.hpp"
#include "GenericDBCDataset.h"

namespace core {

	using VanillaAnimationDataDataset = GenericDBCDataset<DatasetAnimationData, VanillaAnimationDataRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\AnimationData.dbc")>::value>;

	using VanillaCharRacesDataset = GenericDBCDataset<DatasetCharacterRaces, VanillaCharRacesRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\ChrRaces.dbc")>::value>;

	using VanillaCharacterFacialHairStylesDataset = GenericDBCDataset<DatasetCharacterFacialHairStyles, VanillaCharacterFacialHairStylesRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\CharacterFacialHairStyles.dbc")>::value>;

	using VanillaCharHairGeosetsDataset = GenericDBCDataset<DatasetCharacterHairGeosets, VanillaCharHairGeosetsRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\CharHairGeosets.dbc")>::value>;

	using VanillaCharSectionsDataset = GenericDBCDataset<DatasetCharacterSections, VanillaCharSectionsRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\CharSections.dbc")>::value>;

	using VanillaCreatureModelDataDataset = GenericDBCDataset<DatasetCreatureModelData, VanillaCreatureModelDataRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\CreatureModelData.dbc")>::value>;

	class VanillaCreatureDisplayDataset : public DatasetCreatureDisplay, public DBCBackedDataset<VanillaCreatureDisplayInfoRecordAdaptor, CreatureDisplayRecordAdaptor, false> {
	public:
		using Adaptor = VanillaCreatureDisplayInfoRecordAdaptor;
		VanillaCreatureDisplayDataset(MPQFileSystem* fs) :
			DatasetCreatureDisplay(),
			DBCBackedDataset<VanillaCreatureDisplayInfoRecordAdaptor, CreatureDisplayRecordAdaptor, false>(fs, "DBFilesClient\\CreatureDisplayInfo.dbc") {

			dbc_extra = std::make_unique<DBCFile<VanillaDBCCreatureDisplayInfoExtraRecord>>("DBFilesClient\\CreatureDisplayInfoExtra.dbc");
			dbc_extra->open(fs);

			auto& extra_records = dbc_extra->getRecords();
			auto& records = dbc->getRecords();
			for (auto it = records.begin(); it != records.end(); ++it) {

				VanillaDBCCreatureDisplayInfoExtraRecord* extra = nullptr;

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
		VanillaCreatureDisplayDataset(VanillaCreatureDisplayDataset&&) = default;
		virtual ~VanillaCreatureDisplayDataset() {}

		const std::vector<CreatureDisplayRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<CreatureDisplayRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::unique_ptr<DBCFile<VanillaDBCCreatureDisplayInfoExtraRecord>> dbc_extra;
	};


	class VanillaItemDataset : public DatasetItems, protected ReferenceSourceItemsCache {
	public:
		using Adaptor = VanillaItemRecordAdaptor;

		VanillaItemDataset(MPQFileSystem* fs, QString itemReferenceFileName) :
			DatasetItems(),
			ReferenceSourceItemsCache(itemReferenceFileName) {

			for (auto it = itemCacheRecords.begin(); it != itemCacheRecords.end(); ++it) {
				adaptors.push_back(
					std::make_unique<Adaptor>(&(*it))
				);
			}

		}
		VanillaItemDataset(VanillaItemDataset&&) = default;
		virtual ~VanillaItemDataset() {}

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::vector<std::unique_ptr<Adaptor>> adaptors;
	};

	using VanillaItemDisplayInfoDataset = GenericDBCDataset<DatasetItemDisplay, VanillaItemDisplayInfoRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\ItemDisplayInfo.dbc")>::value>;

	using VanillaItemVisualDataset = GenericDBCDataset<DatasetItemVisual, VanillaItemVisualRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\ItemVisuals.dbc")>::value>;

	using VanillaItemVisualEffectDataset = GenericDBCDataset<DatasetItemVisualEffect, VanillaItemVisualEffectRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\ItemVisualEffects.dbc")>::value>;

	using VanillaSpellItemEnchantmentDataset = GenericDBCDataset<DatasetSpellItemEnchantment, VanillaSpellItemEnchantmentRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("DBFilesClient\\SpellItemEnchantment.dbc")>::value>;

};