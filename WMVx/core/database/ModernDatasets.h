#pragma once

#include "GameDataset.h"
#include "ModernDatasetAdaptors.h"
#include "DB2File.h"
#include "DB2BackedDataset.h"

#define BOOST_METAPARSE_LIMIT_STRING_SIZE 64

#include "boost/mpl/string.hpp"
#include "boost/metaparse/string.hpp"

#include "GenericDB2Dataset.h"

namespace core {


	template
		<class T_Adaptor, class T_CharComponentTextureSectionsRecord, class T_CharComponentTextureLayoutsRecord>
	class ModernCharacterComponentTextureDataset : public DatasetCharacterComponentTextures {
	public:
		ModernCharacterComponentTextureDataset(CascFileSystem* fs) {
			db2_sections = std::make_unique<DB2File<T_CharComponentTextureSectionsRecord>>("dbfilesclient/charcomponenttexturesections.db2");
			db2_layouts = std::make_unique<DB2File<T_CharComponentTextureLayoutsRecord>>("dbfilesclient/charcomponenttexturelayouts.db2");

			db2_sections->open(fs);
			db2_layouts->open(fs);

			for (auto it = db2_layouts->cbegin(); it != db2_layouts->cend(); ++it) {
				adaptors.push_back(
					std::make_unique<T_Adaptor>(&(*it), findSections(it->data.id))
				);
			}
		}

		ModernCharacterComponentTextureDataset(ModernCharacterComponentTextureDataset&&) = default;
		virtual ~ModernCharacterComponentTextureDataset() { }

		const std::vector<CharacterComponentTextureAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<CharacterComponentTextureAdaptor*>&>(adaptors);
		}

	protected:
		std::unique_ptr<DB2File<T_CharComponentTextureSectionsRecord>> db2_sections;
		std::unique_ptr<DB2File<T_CharComponentTextureLayoutsRecord>> db2_layouts;

		std::vector<std::unique_ptr<T_Adaptor>> adaptors;

		const std::vector<const T_CharComponentTextureSectionsRecord*> findSections(uint32_t layoutId) const {
			auto sections = std::vector<const T_CharComponentTextureSectionsRecord*>();

			for (auto it = db2_sections->cbegin(); it != db2_sections->cend(); ++it) {
				if (it->data.charComponentTexturelayoutId == layoutId) {
					sections.push_back(&(*it));
				}
			}

			return sections;
		}
	};



	template
		<class T_ItemDisplayInfoRecordAdaptor, class T_ItemDisplayInfoMaterialResRecord>
	class ModernItemDisplayInfoDataset : public DatasetItemDisplay, public DB2BackedDataset<typename T_ItemDisplayInfoRecordAdaptor, ItemDisplayRecordAdaptor, false> {
	public:
		using Adaptor = T_ItemDisplayInfoRecordAdaptor;

		ModernItemDisplayInfoDataset(CascFileSystem* fs, const IFileDataGameDatabase* fdDB) :
			DatasetItemDisplay(),
			DB2BackedDataset<T_ItemDisplayInfoRecordAdaptor, ItemDisplayRecordAdaptor, false>(fs, "dbfilesclient/itemdisplayinfo.db2"),
			fileDataDB(fdDB)
		{
			itemInfoMaterials_db2 = std::make_unique<DB2File<T_ItemDisplayInfoMaterialResRecord>>("dbfilesclient/itemdisplayinfomaterialres.db2");
			itemInfoMaterials_db2->open(fs);

			const auto& sections = this->db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {
				for (auto it2 = it->records.cbegin(); it2 != it->records.cend(); ++it2) {
					auto materials = findMaterials(it2->data.id);

					this->adaptors.push_back(
						std::make_unique<Adaptor>(&(*it2), this->db2.get(), &it->view, materials, fileDataDB)
					);
				}
			}
		}
		ModernItemDisplayInfoDataset(ModernItemDisplayInfoDataset&&) = default;
		virtual ~ModernItemDisplayInfoDataset() {}

		const std::vector<ItemDisplayRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemDisplayRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		const IFileDataGameDatabase* fileDataDB;

		std::unique_ptr<DB2File<T_ItemDisplayInfoMaterialResRecord>> itemInfoMaterials_db2;

		std::vector<const T_ItemDisplayInfoMaterialResRecord*> findMaterials(uint32_t display_info_id) {
			std::vector<const T_ItemDisplayInfoMaterialResRecord*> result;

			const auto& sections = itemInfoMaterials_db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {

				std::mutex mut;
				std::for_each(std::execution::par, it->records.cbegin(), it->records.cend(), [&result, &mut, display_info_id](const T_ItemDisplayInfoMaterialResRecord& rec) {
					if (rec.data.itemDisplayInfoId == display_info_id) {
						std::scoped_lock lock(mut);
						result.push_back(&rec);
					}
					});
			}

			return result;
		}
	};

	template
		<class T_ItemRecordAdaptor, class T_ItemSparseRecord, class T_ItemAppearanceRecord, class T_ItemModifiedAppearanceRecord>
	class ModernItemDataset : public DatasetItems, public DB2BackedDataset<typename T_ItemRecordAdaptor, ItemRecordAdaptor, false> {
	public:
		using Adaptor = T_ItemRecordAdaptor;
		ModernItemDataset(CascFileSystem* fs) :
			DatasetItems(),
			DB2BackedDataset<T_ItemRecordAdaptor, ItemRecordAdaptor, false>(fs, "dbfilesclient/item.db2")
		{

			itemSparse_db2 = std::make_unique<DB2File<T_ItemSparseRecord>>("dbfilesclient/itemsparse.db2");
			itemAppearance_db2 = std::make_unique<DB2File<T_ItemAppearanceRecord>>("dbfilesclient/itemappearance.db2");
			itemModifiedAppearance_db2 = std::make_unique<DB2File<T_ItemModifiedAppearanceRecord>>("dbfilesclient/itemmodifiedappearance.db2");

			itemSparse_db2->open(fs);
			itemAppearance_db2->open(fs);
			itemModifiedAppearance_db2->open(fs);

			//unsure if main table should be items.db2 or itemsparse.db2 ?

			for (auto it = this->db2->cbegin(); it != this->db2->cend(); ++it) {
				const T_ItemSparseRecord* sparse_record = findSparseItemById(it->data.id);

				if (sparse_record == nullptr) {
					continue;
				}

				auto appearanceModifiers = findAppearanceModifiers(it->data.id);
				//TODO handle multiple appearances
				size_t modifier_count = appearanceModifiers.size();
				if (modifier_count == 1) {
					const T_ItemAppearanceRecord* appearance_record = findAppearance(appearanceModifiers[0]->data.itemAppearanceId);

					if (appearance_record != nullptr) {
						this->adaptors.push_back(
							std::make_unique<Adaptor>(&(*it), this->db2.get(), &(it.section()), sparse_record, appearance_record)
						);
					}
				}
			}
		}
		ModernItemDataset(ModernItemDataset&&) = default;
		virtual ~ModernItemDataset() { }

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->adaptors);
		}

	protected:
		std::unique_ptr<DB2File<T_ItemSparseRecord>> itemSparse_db2;
		std::unique_ptr<DB2File<T_ItemAppearanceRecord>> itemAppearance_db2;
		std::unique_ptr<DB2File<T_ItemModifiedAppearanceRecord>> itemModifiedAppearance_db2;

		const T_ItemSparseRecord* findSparseItemById(uint32_t itemId) {

			const auto& sections = itemSparse_db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {
				auto par_result = std::find_if(std::execution::par, std::begin(it->records), std::end(it->records), [itemId](const T_ItemSparseRecord& sparse_record) {
					return sparse_record.data.id == itemId;
					});

				if (par_result != it->records.end()) {
					return &(*par_result);
				}
			}

			return nullptr;
		}


		std::vector<const T_ItemModifiedAppearanceRecord*> findAppearanceModifiers(uint32_t itemId) {
			std::vector<const T_ItemModifiedAppearanceRecord*> result;

			const auto& sections = itemModifiedAppearance_db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {

				std::mutex mut;
				std::for_each(std::execution::par, it->records.cbegin(), it->records.cend(), [&result, &mut, itemId](const T_ItemModifiedAppearanceRecord& rec) {
					if (rec.data.itemId == itemId) {
						std::scoped_lock lock(mut);
						result.push_back(&rec);
					}
					});
			}

			return result;
		}

		const T_ItemAppearanceRecord* findAppearance(uint32_t appearanceId) {

			const auto& sections = itemAppearance_db2->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {

				auto result = std::find_if(std::execution::par, it->records.cbegin(), it->records.cend(), [appearanceId](const T_ItemAppearanceRecord& record) {
					return record.data.id == appearanceId;
					});

				if (result != it->records.cend()) {
					return &(*result);
				}
			}

			return nullptr;
		}
	};
};