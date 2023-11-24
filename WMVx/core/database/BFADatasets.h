#pragma once
#include "GameDataset.h"
#include "BFADatasetAdaptors.h"
#include "DB2File.h"
#include "BFARecordDefinitions.h"
#include "DB2BackedDataset.h"

#define BOOST_METAPARSE_LIMIT_STRING_SIZE 64

#include "boost/mpl/string.hpp"
#include "boost/metaparse/string.hpp"

#include "GenericDB2Dataset.h"
#include "ReferenceSource.h"

#include "ModernDatasets.h"

#include <algorithm>
#include <execution>

namespace core {

	class BFAAnimationDataDataset : public DatasetAnimationData, public DB2BackedDataset<BFAAnimationDataRecordAdaptor, AnimationDataRecordAdaptor, false>,
		protected ReferenceSourceAnimationNames
	{
	public:
		using Adaptor = BFAAnimationDataRecordAdaptor;

		BFAAnimationDataDataset(CascFileSystem* fs, QString animationReferenceFileName) : 
			DatasetAnimationData(),
			DB2BackedDataset<BFAAnimationDataRecordAdaptor, AnimationDataRecordAdaptor, false>(fs, "dbfilesclient/animationdata.db2"),
			ReferenceSourceAnimationNames(animationReferenceFileName) {

			for (auto it = db2->cbegin(); it != db2->cend(); ++it) {
				QString name = animationNames.contains(it->data.id) ? animationNames[it->data.id] : "";
				adaptors.push_back(
					std::make_unique<Adaptor>(&(*it), db2.get(), &(it.section()), name)
				);
			}
		}
		BFAAnimationDataDataset(BFAAnimationDataDataset&&) = default;
		virtual ~BFAAnimationDataDataset() {}

		const std::vector<AnimationDataRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<AnimationDataRecordAdaptor*>&>(this->adaptors);
		}

	protected:
	};

	using BFACharRacesDataset = GenericDB2Dataset<DatasetCharacterRaces, BFACharRacesRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("dbfilesclient/chrraces.db2")>::value>;

	using BFACharacterFacialHairStylesDataset = GenericDB2Dataset<DatasetCharacterFacialHairStyles, BFACharacterFacialHairStylesRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("dbfilesclient/characterfacialhairstyles.db2")>::value>;
	
	using BFACharHairGeosetsDataset = GenericDB2Dataset<DatasetCharacterHairGeosets, BFACharHairGeosetsRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("dbfilesclient/charhairgeosets.db2")>::value>;
	
	using BFACreatureModelDataDataset = GenericDB2Dataset<DatasetCreatureModelData, BFACreatureModelDataRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("dbfilesclient/creaturemodeldata.db2")>::value >;

	using BFACreatureDisplayDataset = GenericDB2Dataset<DatasetCreatureDisplay, BFACreatureDisplayRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("dbfilesclient/creaturedisplayinfo.db2")>::value >;

	using BFANPCsDataset = GenericDB2Dataset<DatasetNPCs, BFANPCRecordAdaptor, boost::mpl::c_str<BOOST_METAPARSE_STRING("dbfilesclient/creature.db2")>::value >;

	using BFAItemDisplayInfoDataset = ModernItemDisplayInfoDataset< BFAItemDisplayInfoRecordAdaptor, BFADB2ItemDisplayInfoMaterialResRecord>;

	using BFAItemDataset = ModernItemDataset<BFAItemRecordAdaptor, BFADB2ItemSparseRecord, BFADB2ItemAppearanceRecord, BFADB2ItemModifiedAppearanceRecord>;

	using BFACharacterComponentTextureDataset = ModernCharacterComponentTextureDataset<BFACharacterComponentTextureAdaptor, BFADB2CharComponentTextureSectionsRecord, BFADB2CharComponentTextureLayoutsRecord>;

	class BFACharSectionsDataset : public DatasetCharacterSections, public DB2BackedDataset<BFACharSectionsRecordAdaptor, CharacterSectionRecordAdaptor, false> {
	public:
		using Adaptor = BFACharSectionsRecordAdaptor;
		BFACharSectionsDataset(CascFileSystem* fs, const IFileDataGameDatabase* fdDB) :
			DatasetCharacterSections(),
			DB2BackedDataset<BFACharSectionsRecordAdaptor, CharacterSectionRecordAdaptor, false>(fs, "dbfilesclient/charsections.db2"),
			fileDataDB(fdDB)
		{	
			db2_base_sections = std::make_unique<DB2File<BFADB2CharBaseSectionRecord>>("dbfilesclient/charbasesection.db2");
			db2_base_sections->open(fs);

			for (auto it = db2->cbegin(); it != db2->cend(); ++it) {
				adaptors.push_back(
					std::make_unique<Adaptor>(&(*it), db2.get(), &it.section(), fileDataDB, findBase(it->data.baseSectionId))
				);
			}
		}
		BFACharSectionsDataset(BFACharSectionsDataset&&) = default;
		virtual ~BFACharSectionsDataset() {}

		const std::vector<CharacterSectionRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<CharacterSectionRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::unique_ptr<DB2File<BFADB2CharBaseSectionRecord>> db2_base_sections;
		const IFileDataGameDatabase* fileDataDB;

		const BFADB2CharBaseSectionRecord* findBase(uint32_t baseSectionId) {
			for (auto it = db2_base_sections->cbegin(); it != db2_base_sections->cend(); ++it) {
				if (it->data.id == baseSectionId) {
					return &(*it);
				}
			}
			
			return nullptr;
		}

	};


};