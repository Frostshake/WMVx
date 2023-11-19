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

			const auto& sections = db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {
				for (auto it2 = it->records.cbegin(); it2 != it->records.cend(); ++it2) {

					QString name = animationNames.contains(it2->data.id) ? animationNames[it2->data.id] : "";

					adaptors.push_back(
						std::make_unique<Adaptor>(&(*it2), db2.get(), &it->view, name)
					);
				}
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

	class BFACharSectionsDataset : public DatasetCharacterSections, public DB2BackedDataset<BFACharSectionsRecordAdaptor, CharacterSectionRecordAdaptor, false> {
	public:
		using Adaptor = BFACharSectionsRecordAdaptor;
		BFACharSectionsDataset(CascFileSystem* fs, const FileDataGameDatabase* fdDB) :
			DatasetCharacterSections(),
			DB2BackedDataset<BFACharSectionsRecordAdaptor, CharacterSectionRecordAdaptor, false>(fs, "dbfilesclient/charsections.db2"),
			fileDataDB(fdDB)
		{	
			db2_base_sections = std::make_unique<DB2File<BFADB2CharBaseSectionRecord>>("dbfilesclient/charbasesection.db2");

			db2_base_sections->open(fs);

			const auto& sections = db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {
				for (auto it2 = it->records.cbegin(); it2 != it->records.cend(); ++it2) {
					adaptors.push_back(
						std::make_unique<Adaptor>(&(*it2), db2.get(), &it->view, fileDataDB, findBase(it2->data.baseSectionId))
					);
				}
			}
		}
		BFACharSectionsDataset(BFACharSectionsDataset&&) = default;
		virtual ~BFACharSectionsDataset() {}

		const std::vector<CharacterSectionRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<CharacterSectionRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::unique_ptr<DB2File<BFADB2CharBaseSectionRecord>> db2_base_sections;
		const FileDataGameDatabase* fileDataDB;

		const BFADB2CharBaseSectionRecord* findBase(uint32_t baseSectionId) {
			
			const auto& sections = db2_base_sections->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {
				for (auto it2 = it->records.cbegin(); it2 != it->records.cend(); ++it2) {
					if (it2->data.id == baseSectionId) {
						return &(*it2);
					}
				}
			}
			return nullptr;
		}

	};

	class BFACharacterComponentTextureDataset : public DatasetCharacterComponentTextures {
	public:
		BFACharacterComponentTextureDataset(CascFileSystem* fs) {
			db2_sections = std::make_unique<DB2File<BFADB2CharComponentTextureSectionsRecord>>("dbfilesclient/charcomponenttexturesections.db2");
			db2_layouts = std::make_unique<DB2File<BFADB2CharComponentTextureLayoutsRecord>>("dbfilesclient/charcomponenttexturelayouts.db2");

			db2_sections->open(fs);
			db2_layouts->open(fs);
			
			const auto& layout_sections = db2_layouts->getSections();
			for (auto it = layout_sections.begin(); it != layout_sections.end(); ++it) {
				for (auto it2 = it->records.cbegin(); it2 != it->records.cend(); ++it2) {
					adaptors.push_back(
						std::make_unique<BFACharacterComponentTextureAdaptor>(&(*it2), findSections(it2->data.id))
					);
				}
			}
		}

		BFACharacterComponentTextureDataset(BFACharacterComponentTextureDataset&&) = default;
		virtual ~ BFACharacterComponentTextureDataset() { }

		const std::vector<CharacterComponentTextureAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<CharacterComponentTextureAdaptor*>&>(adaptors);
		}

	protected: 
		std::unique_ptr<DB2File<BFADB2CharComponentTextureSectionsRecord>> db2_sections;
		std::unique_ptr<DB2File<BFADB2CharComponentTextureLayoutsRecord>> db2_layouts;

		std::vector<std::unique_ptr<BFACharacterComponentTextureAdaptor>> adaptors;

		const std::vector<const BFADB2CharComponentTextureSectionsRecord*> findSections(uint32_t layoutId) const {
			auto sections = std::vector<const BFADB2CharComponentTextureSectionsRecord*>();

			const auto& section_sections = db2_sections->getSections();
			for (auto it = section_sections.begin(); it != section_sections.end(); ++it) {
				for (auto it2 = it->records.cbegin(); it2 != it->records.cend(); ++it2) {
					if (it2->data.charComponentTexturelayoutId == layoutId) {
						sections.push_back(&(*it2));
					}
				}
			}

			return sections;
		}
	};

};