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

	class BFAItemDisplayInfoDataset : public DatasetItemDisplay, public DB2BackedDataset<BFAItemDisplayInfoRecordAdaptor, ItemDisplayRecordAdaptor, false> {
	public:
		using Adaptor = BFAItemDisplayInfoRecordAdaptor;

		BFAItemDisplayInfoDataset(CascFileSystem* fs, 
			DB2File<BFADB2ModelFileDataRecord>* modelFileDataDB,
			DB2File<BFADB2TextureFileDataRecord>* textureFileDataDB) :
			DatasetItemDisplay(),
			DB2BackedDataset<BFAItemDisplayInfoRecordAdaptor, ItemDisplayRecordAdaptor, false>(fs, "dbfilesclient/itemdisplayinfo.db2"),
			modelFileDataDB(modelFileDataDB),
			textureFileDataDB(textureFileDataDB)
		{
			itemInfoMaterials_db2 = std::make_unique<DB2File<BFADB2ItemDisplayInfoMaterialResRecord>>("dbfilesclient/itemdisplayinfomaterialres.db2");
			itemInfoMaterials_db2->open(fs);

			const auto& sections = db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {
				for (auto it2 = it->records.cbegin(); it2 != it->records.cend(); ++it2) {
					auto materials = findMaterials(it2->data.id);

					adaptors.push_back(
						std::make_unique<Adaptor>(&(*it2), db2.get(), &it->view, materials, modelFileDataDB, textureFileDataDB)
					);
				}
			}
		}
		BFAItemDisplayInfoDataset(BFAItemDisplayInfoDataset&&) = default;
		virtual ~BFAItemDisplayInfoDataset() {}

		const std::vector<ItemDisplayRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemDisplayRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		const DB2File<BFADB2ModelFileDataRecord>* modelFileDataDB;
		const DB2File<BFADB2TextureFileDataRecord>* textureFileDataDB;

		std::unique_ptr<DB2File<BFADB2ItemDisplayInfoMaterialResRecord>> itemInfoMaterials_db2;

		std::vector<const BFADB2ItemDisplayInfoMaterialResRecord*> findMaterials(uint32_t display_info_id) {
			std::vector<const BFADB2ItemDisplayInfoMaterialResRecord*> result;

			const auto& sections = itemInfoMaterials_db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {

				std::mutex mut;
				std::for_each(std::execution::par, it->records.cbegin(), it->records.cend(), [&result, &mut, display_info_id](const BFADB2ItemDisplayInfoMaterialResRecord& rec) {
					if (rec.data.itemDisplayInfoId == display_info_id) {
						std::scoped_lock lock(mut);
						result.push_back(&rec);
					}
				});
			}

			return result;
		}
	};

	class BFAItemDataset : public DatasetItems, public DB2BackedDataset<BFAItemRecordAdaptor, ItemRecordAdaptor, false> {
	public:
		using Adaptor = BFAItemRecordAdaptor;
		BFAItemDataset(CascFileSystem* fs) : 
			DatasetItems(),
			DB2BackedDataset<BFAItemRecordAdaptor, ItemRecordAdaptor, false>(fs, "dbfilesclient/item.db2")
		{

			itemSparse_db2 = std::make_unique<DB2File<BFADB2ItemSparseRecord>>("dbfilesclient/itemsparse.db2");
			itemAppearance_db2 = std::make_unique<DB2File<BFADB2ItemAppearanceRecord>>("dbfilesclient/itemappearance.db2");
			itemModifiedAppearance_db2 = std::make_unique<DB2File<BFADB2ItemModifiedAppearanceRecord>>("dbfilesclient/itemmodifiedappearance.db2");

			itemSparse_db2->open(fs);	
			itemAppearance_db2->open(fs);
			itemModifiedAppearance_db2->open(fs);

			//unsure if main table should be items.db2 or itemsparse.db2 ?

			const auto& sections = db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {
				for (auto it2 = it->records.cbegin(); it2 != it->records.cend(); ++it2) {

					const BFADB2ItemSparseRecord* sparse_record = findSparseItemById(it2->data.id);

					if (sparse_record == nullptr) {
						continue;
					}

					auto appearanceModifiers = findAppearanceModifiers(it2->data.id);
					//TODO handle multiple appearances
					size_t modifier_count = appearanceModifiers.size();
					if (modifier_count == 1) {
						const BFADB2ItemAppearanceRecord* appearance_record = findAppearance(appearanceModifiers[0]->data.itemAppearanceId);

						if (appearance_record != nullptr) {
							adaptors.push_back(
								std::make_unique<Adaptor>(&(*it2), db2.get(), &it->view, sparse_record, appearance_record)
							);
						}
					}
				}
			}
		}
		BFAItemDataset(BFAItemDataset&&) = default;
		virtual ~BFAItemDataset(){ }

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->adaptors);
		}

		protected:
			std::unique_ptr<DB2File<BFADB2ItemSparseRecord>> itemSparse_db2;
			std::unique_ptr<DB2File<BFADB2ItemAppearanceRecord>> itemAppearance_db2;
			std::unique_ptr<DB2File<BFADB2ItemModifiedAppearanceRecord>> itemModifiedAppearance_db2;

			const BFADB2ItemSparseRecord* findSparseItemById(uint32_t itemId) {

				const auto& sections = itemSparse_db2->getSections();
				for (auto it = sections.begin(); it != sections.end(); ++it) {
					auto par_result = std::find_if(std::execution::par, std::begin(it->records), std::end(it->records), [itemId](const BFADB2ItemSparseRecord& sparse_record) {
						return sparse_record.data.id == itemId;
					});
						
					if (par_result != it->records.end()) {
						return &(*par_result);
					}
				}

				return nullptr;
			}


			std::vector<const BFADB2ItemModifiedAppearanceRecord*> findAppearanceModifiers(uint32_t itemId) {
				std::vector<const BFADB2ItemModifiedAppearanceRecord*> result;

				const auto& sections = itemModifiedAppearance_db2->getSections();
				for (auto it = sections.begin(); it != sections.end(); ++it) {

					std::mutex mut;
					std::for_each(std::execution::par, it->records.cbegin(), it->records.cend(), [&result, &mut, itemId](const BFADB2ItemModifiedAppearanceRecord& rec) {
						if (rec.data.itemId == itemId) {
							std::scoped_lock lock(mut);
							result.push_back(&rec);
						}
					});
				}

				return result;
			}

			const BFADB2ItemAppearanceRecord* findAppearance(uint32_t appearanceId) {

				const auto& sections = itemAppearance_db2->getSections();
				for (auto it = sections.begin(); it != sections.end(); ++it) {

					auto result = std::find_if(std::execution::par, it->records.cbegin(), it->records.cend(), [appearanceId](const BFADB2ItemAppearanceRecord& record) {
						return record.data.id == appearanceId;
					});

					if (result != it->records.cend()) {
						return &(*result);
					}
				}

				return nullptr;
			}
	};

	class BFACharSectionsDataset : public DatasetCharacterSections, public DB2BackedDataset<BFACharSectionsRecordAdaptor, CharacterSectionRecordAdaptor, false> {
	public:
		using Adaptor = BFACharSectionsRecordAdaptor;
		BFACharSectionsDataset(CascFileSystem* fs, const DB2File<BFADB2TextureFileDataRecord>* textureFileDataDB) :
			DatasetCharacterSections(),
			DB2BackedDataset<BFACharSectionsRecordAdaptor, CharacterSectionRecordAdaptor, false>(fs, "dbfilesclient/charsections.db2"),
			textureFileDataDB(textureFileDataDB)
		{	
			db2_base_sections = std::make_unique<DB2File<BFADB2CharBaseSectionRecord>>("dbfilesclient/charbasesection.db2");

			db2_base_sections->open(fs);

			const auto& sections = db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {
				for (auto it2 = it->records.cbegin(); it2 != it->records.cend(); ++it2) {
					adaptors.push_back(
						std::make_unique<Adaptor>(&(*it2), db2.get(), &it->view, textureFileDataDB, findBase(it2->data.baseSectionId))
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
		const DB2File<BFADB2TextureFileDataRecord>* textureFileDataDB;

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