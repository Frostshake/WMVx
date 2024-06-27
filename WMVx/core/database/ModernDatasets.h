#pragma once

#include "GameDataset.h"
#include "ModernDatasetAdaptors.h"
#include "GenericDB2Dataset.h"
#include "../filesystem/CascFileSystem.h"

namespace core {

	template<typename ImplAdaptor>
	class ModernAnimationDataDataset : public DatasetAnimationData, protected ReferenceSourceAnimationNames
	{
	public:
		using Adaptor = ImplAdaptor;

		ModernAnimationDataDataset(CascFileSystem* fs, const GameFileUri& uri, const QString& animationReferenceFileName) :
			DatasetAnimationData(), ReferenceSourceAnimationNames(animationReferenceFileName)
		{
			auto file = fs->openFile(uri);
			auto db2 = WDBReader::Database::makeDB2File<ImplAdaptor::Record>(
				static_cast<CascFile*>(file.get())->release()
			);
			
			_adaptors.reserve(db2->size());

			for (auto& rec : *db2) {
				if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
					auto found = animationNames.find(rec.data.id);
					QString name = "";
					if (found != animationNames.end()) {
						name = found->second;
					}

					_adaptors.push_back(
						std::make_unique<ImplAdaptor>(std::move(rec), std::move(name))
					);
				}
			}
		}

		const std::vector<AnimationDataRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<AnimationDataRecordAdaptor*>&>(this->_adaptors);
		}

	protected:
		std::vector<std::unique_ptr<ImplAdaptor>> _adaptors;
	};

	template<class T_Adaptor, WDBReader::Database::TRecord T_LayoutsRecord, WDBReader::Database::TRecord T_SectionsRecord>
		class ModernCharacterComponentTextureDataset : public DatasetCharacterComponentTextures {
		public:
			using Adaptor = T_Adaptor;
			ModernCharacterComponentTextureDataset(CascFileSystem* fs) : DatasetCharacterComponentTextures() {
				
				{
					auto sections_file = fs->openFile("dbfilesclient/charcomponenttexturesections.db2");
					auto db2 = WDBR::Database::makeDB2File<T_SectionsRecord>(
						static_cast<CascFile*>(sections_file.get())->release()
					);

					_sections.reserve(db2->size());

					for (auto& rec : *db2) {
						if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
							_sections.push_back(std::move(rec));
						}
					}
				}

				{
					auto layouts_file = fs->openFile("dbfilesclient/charcomponenttexturelayouts.db2");
					auto db2 = WDBR::Database::makeDB2File<T_LayoutsRecord>(
						static_cast<CascFile*>(layouts_file.get())->release()
					);

					_adaptors.reserve(db2->size());

					for (auto& rec : *db2) {
						if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
							auto sections = findSections(rec.data.id);
							_adaptors.push_back(
								std::make_unique<T_Adaptor>(std::move(rec), std::move(sections))
							);
						}
					}
				}
			}

			ModernCharacterComponentTextureDataset(ModernCharacterComponentTextureDataset&&) = default;
			virtual ~ModernCharacterComponentTextureDataset() = default;

			const std::vector<CharacterComponentTextureAdaptor*>& all() const override {
				return reinterpret_cast<const std::vector<CharacterComponentTextureAdaptor*>&>(_adaptors);
			}

		protected:
			std::vector<T_SectionsRecord> _sections;
			std::vector<std::unique_ptr<T_Adaptor>> _adaptors;

			std::vector<const T_SectionsRecord*> findSections(uint32_t layoutId) const {
				auto sections = std::vector<const T_SectionsRecord*>();

				for (auto it = _sections.cbegin(); it != _sections.cend(); ++it) {
					if (it->data.charComponentTextureLayoutId == layoutId) {
						sections.push_back(&(*it));
					}
				}

				return sections;
			}
	};


	template<class T_Adaptor, class T_ExtraAdaptor = void>
	class ModernCreatureDisplayDataset : public DatasetCreatureDisplay {
	public:
		using Adaptor = T_Adaptor;
		ModernCreatureDisplayDataset(CascFileSystem* fs) : DatasetCreatureDisplay()
		{
			std::unordered_map<uint32_t, std::unique_ptr<CreatureDisplayExtraRecordAdaptor>> extras;
			if constexpr (!std::is_same_v<T_ExtraAdaptor, void>)
			{
				{
					auto extras_file = fs->openFile("dbfilesclient/creaturedisplayinfoextra.db2");
					auto db2 = WDBR::Database::makeDB2File<T_ExtraAdaptor::Record>(
						static_cast<CascFile*>(extras_file.get())->release()
					);

					for (auto& rec : *db2) {
						if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
							const auto id = rec.data.id;
							extras[id] = std::make_unique<T_ExtraAdaptor>(std::move(rec));
						}
					}
				}
			}

			{
				auto info_file = fs->openFile("dbfilesclient/creaturedisplayinfo.db2");
				auto db2 = WDBR::Database::makeDB2File<T_Adaptor::Record>(
					static_cast<CascFile*>(info_file.get())->release()
				);

				_adaptors.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						std::unique_ptr<CreatureDisplayExtraRecordAdaptor> extra = nullptr;

						if constexpr (!std::is_same_v<T_ExtraAdaptor, void>)
						{
							if (rec.data.extendedDisplayInfoId > 0) {
								auto found = extras.find(rec.data.extendedDisplayInfoId);
								if (found != extras.end()) {
									extra = std::move(found->second);
									extras.erase(found);
								}
							}
						}

						_adaptors.push_back(
							std::make_unique<T_Adaptor>(std::move(rec), std::move(extra))
						);
					}
				}
			}
		}

		const std::vector<CreatureDisplayRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<CreatureDisplayRecordAdaptor*>&>(this->_adaptors);
		}

	protected:
		std::vector<std::unique_ptr<T_Adaptor>> _adaptors;

	};

	template
		<class T_ItemRecordAdaptor, WDBReader::Database::TRecord T_ItemSparseRecord, WDBReader::Database::TRecord T_ItemAppearanceRecord, WDBReader::Database::TRecord T_ItemModifiedAppearanceRecord>
	class ModernItemDataset : public DatasetItems {
	public:
		using Adaptor = T_ItemRecordAdaptor;
		ModernItemDataset(CascFileSystem* fs) : DatasetItems()
		{

			std::unordered_map<uint32_t, const T_ItemSparseRecord*> sparse_map;
			std::unordered_map<uint32_t, const T_ItemAppearanceRecord*> appearance_map;
			std::unordered_multimap<uint32_t, T_ItemModifiedAppearanceRecord> appearance_modifiers;

			{
				auto sparse_file = fs->openFile("dbfilesclient/itemsparse.db2");
				auto db2 = WDBR::Database::makeDB2File<T_ItemSparseRecord>(
					static_cast<CascFile*>(sparse_file.get())->release()
				);

				_sparse.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						const auto id = rec.data.id;
						_sparse.push_back(std::move(rec));
						sparse_map.emplace(id, &_sparse.back());
					}
				}
			}


			{
				auto appearance_file = fs->openFile("dbfilesclient/itemappearance.db2");
				auto db2 = WDBR::Database::makeDB2File<T_ItemAppearanceRecord>(
					static_cast<CascFile*>(appearance_file.get())->release()
				);

				_appearance.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						const auto id = rec.data.id;
						_appearance.push_back(std::move(rec));
						appearance_map.emplace(id, &_appearance.back());
					}
				}
			}


			{
				auto modifiers_file = fs->openFile("dbfilesclient/itemmodifiedappearance.db2");
				auto db2 = WDBR::Database::makeDB2File<T_ItemModifiedAppearanceRecord>(
					static_cast<CascFile*>(modifiers_file.get())->release()
				);

				for (auto& rec : *db2) {
					const auto item_id = rec.data.itemId;
					appearance_modifiers.emplace(item_id, std::move(rec));
				}
			}

			{
				auto items_file = fs->openFile("dbfilesclient/item.db2");
				auto db2 = WDBR::Database::makeDB2File<T_ItemRecordAdaptor::Record>(
					static_cast<CascFile*>(items_file.get())->release()
				);

				_adaptors.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						const auto id = rec.data.id;
						const T_ItemSparseRecord* sparse_record = nullptr;

						auto sparse_found = sparse_map.find(id);
						if (sparse_found != sparse_map.end()) {
							sparse_record = sparse_found->second;
						}

						if (sparse_record == nullptr) {
							continue;
						}

						std::vector<uint32_t> itemAppearanceIds;
						auto range = appearance_modifiers.equal_range(id);
						for (auto it = range.first; it != range.second; ++it) {
							itemAppearanceIds.push_back(it->second.data.itemAppearanceId);
						}

						//TODO handle multiple appearances
						if (itemAppearanceIds.size() == 1) {
							const T_ItemAppearanceRecord* appearance_record = nullptr;

							auto appearance_found = appearance_map.find(itemAppearanceIds[0]);
							if (appearance_found != appearance_map.end())
							{
								appearance_record = appearance_found->second;
							}

							if (appearance_record != nullptr) {

								_adaptors.push_back(
									std::make_unique<T_ItemRecordAdaptor>(std::move(rec), sparse_record, appearance_record)
								);

							}
						}
					}
				}
			}
		}
		ModernItemDataset(ModernItemDataset&&) = default;
		virtual ~ModernItemDataset() = default;

		const std::vector<ItemRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->_adaptors);
		}

	protected:
		std::vector<std::unique_ptr<T_ItemRecordAdaptor>> _adaptors;
		std::vector<T_ItemSparseRecord> _sparse;
		std::vector<T_ItemAppearanceRecord> _appearance;
	};


	template<class T_Adaptor, WDBReader::Database::TRecord T_MatResRecord>
	class ModernItemDisplayInfoDataset : public DatasetItemDisplay {
	public:
		using Adaptor = T_Adaptor;
		ModernItemDisplayInfoDataset(CascFileSystem* fs, const IFileDataGameDatabase* fdDB) : DatasetItemDisplay()
		{

			std::unordered_multimap<uint32_t, const T_MatResRecord*> materials_map;

			{
				auto materials_file = fs->openFile("dbfilesclient/itemdisplayinfomaterialres.db2");
				auto db2 = WDBR::Database::makeDB2File<T_MatResRecord>(
					static_cast<CascFile*>(materials_file.get())->release()
				);

				_materials.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						_materials.push_back(std::move(rec));
						materials_map.emplace(rec.data.itemDisplayInfoId, &_materials.back());
					}
				}
			}

			{
				auto display_file = fs->openFile("dbfilesclient/itemdisplayinfo.db2");
				auto db2 = WDBR::Database::makeDB2File<T_Adaptor::Record>(
					static_cast<CascFile*>(display_file.get())->release()
				);

				_adaptors.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						std::vector<const T_MatResRecord*> mats;
						auto range = materials_map.equal_range(rec.data.id);
						for (auto it = range.first; it != range.second; ++it) {
							mats.push_back(it->second);
						}
						materials_map.erase(rec.data.id);

						_adaptors.push_back(
							std::make_unique<T_Adaptor>(std::move(rec), std::move(mats), fdDB)
						);
					}
				}
			}
		}
		ModernItemDisplayInfoDataset(ModernItemDisplayInfoDataset&&) = default;
		virtual ~ModernItemDisplayInfoDataset() = default;

		const std::vector<ItemDisplayRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemDisplayRecordAdaptor*>&>(this->_adaptors);
		}

	protected:
		std::vector<std::unique_ptr<T_Adaptor>> _adaptors;
		std::vector<T_MatResRecord> _materials;
	};

};