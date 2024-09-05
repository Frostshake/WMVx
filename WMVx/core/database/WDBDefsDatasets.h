#pragma once
#include "GameDataset.h"
#include "../filesystem/GameFileSystem.h"
#include "ReferenceSource.h"
#include <WDBReader/WoWDBDefs.hpp>
#include <WDBReader/Database/Schema.hpp>
#include "WDBDefsDatasetAdaptors.h"
#include <fstream>


namespace core {

	inline WDBReader::Database::RuntimeSchema make_wbdr_schema(const std::string& name, const WDBReader::GameVersion& version) {
		std::ifstream stream("Support Files/definitions/" + name);
		if (stream.fail()) {
			throw std::runtime_error("Defintion files doesnt exist: " + name);
		}

		auto definition = WDBReader::WoWDBDefs::DBDReader::read(stream);
		auto schema = WDBReader::WoWDBDefs::makeSchema(definition, version);

		if (!schema.has_value()) {
			throw std::runtime_error("Unable to load schema for " + name);
		}

		return *schema;
	}

	template<typename BaseDataset, typename ImplAdaptor>
	class GenericWDBDefsDataset : public BaseDataset {
	public:
		using Adaptor = ImplAdaptor;
		GenericWDBDefsDataset(GameFileSystem* fs, const GameFileUri& uri, const WDBReader::GameVersion& version, const std::string& defName) : BaseDataset()
		{
			auto file = fs->openFile(uri);
			if (file == nullptr) {
				throw std::runtime_error("Unable to open database " + uri.toString().toStdString());
			}

			auto schema = std::make_shared<WDBReader::Database::RuntimeSchema>(
				make_wbdr_schema(defName, version)
			);

			auto db = WDBR::Database::makeDB2File(
				*schema,
				file->release()
			);


			for (auto& rec : *db) {
				if (rec.encryptionState != WDBReader::Database::RecordEncryption::ENCRYPTED) {
					_adaptors.push_back(
						std::make_unique<ImplAdaptor>(schema, std::move(rec))
					);
				}
			}
		}

		const std::vector<typename BaseDataset::BaseAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<typename BaseDataset::BaseAdaptor*>&>(this->_adaptors);
		}

	protected:
		std::vector<std::unique_ptr<ImplAdaptor>> _adaptors;
	};

	template<typename ImplAdaptor>
	class ModernWDBDefsAnimationDataDataset : public DatasetAnimationData, protected ReferenceSourceAnimationNames {
	public:
		using Adaptor = ImplAdaptor;

		ModernWDBDefsAnimationDataDataset(CascFileSystem* fs, const WDBReader::GameVersion& version, const QString& animationReferenceFileName) :
			DatasetAnimationData(), ReferenceSourceAnimationNames(animationReferenceFileName)
		{
			auto file = fs->openFile("dbfilesclient/animationdata.db2");
			auto schema = std::make_shared<WDBReader::Database::RuntimeSchema>(
				make_wbdr_schema("AnimationData.dbd", version)
			);

			auto db2 = WDBReader::Database::makeDB2File(
				*schema,
				file->release()
			);

			_adaptors.reserve(db2->size());

			for (auto& rec : *db2) {
				if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
					auto [id] = (*schema)(rec).get<uint32_t>("ID");
					auto found = animationNames.find(id);
					QString name = "";
					if (found != animationNames.end()) {
						name = found->second;
					}

					_adaptors.push_back(
						std::make_unique<ImplAdaptor>(schema, std::move(rec), std::move(name))
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

	template<typename ImplAdaptor>
	class ModernWDBDefsCharacterComponentTextureDataset : public DatasetCharacterComponentTextures {
	public:
		using Adaptor = ImplAdaptor;
		ModernWDBDefsCharacterComponentTextureDataset(CascFileSystem* fs, const WDBReader::GameVersion& version) : DatasetCharacterComponentTextures() {

			std::unordered_map<uint32_t, std::map<CharacterRegion, CharacterRegionCoords>> sections_map;

			{
				auto sections_file = fs->openFile("dbfilesclient/charcomponenttexturesections.db2");
				auto schema = make_wbdr_schema("CharComponentTextureSections.dbd", version);
				auto db2 = WDBR::Database::makeDB2File(
					schema,
					sections_file->release()
				);

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						CharacterRegionCoords coords;
						CharacterRegion region;
						uint32_t layout_id;
						std::tie(layout_id, region, coords.positionX, coords.positionY, coords.sizeX, coords.sizeY) = 
							schema(rec).get<uint32_t, CharacterRegion, int32_t, int32_t, int32_t, int32_t>(
								"CharComponentTextureLayoutID", "SectionType", "X", "Y", "Width", "Height"
							);

						auto existing = sections_map.find(layout_id);

						if (existing != sections_map.end()) {
							existing->second.emplace(region, coords);
						}
						else {
							sections_map.emplace(layout_id, decltype(sections_map)::mapped_type{
								{region, coords}
							});
						}
					}
				}
			}

			{
				auto layouts_file = fs->openFile("dbfilesclient/charcomponenttexturelayouts.db2");
				auto schema = std::make_shared<WDBReader::Database::RuntimeSchema>(
					make_wbdr_schema("CharComponentTextureLayouts.dbd", version)
				);
				auto db2 = WDBR::Database::makeDB2File(
					*schema,
					layouts_file->release()
				);

				_adaptors.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						auto [id] = (*schema)(rec).get<uint32_t>("ID");
						std::map<CharacterRegion, CharacterRegionCoords> secs;
						auto found = sections_map.find(id);
						if (found != sections_map.end()) {
							secs = std::move(found->second);
							sections_map.erase(found);
						}

						_adaptors.push_back(
							std::make_unique<ImplAdaptor>(schema, std::move(rec), std::move(secs))
						);
					}
				}
			}
		}

		const std::vector<CharacterComponentTextureAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<CharacterComponentTextureAdaptor*>&>(_adaptors);
		}

	protected:
		std::vector<std::unique_ptr<ImplAdaptor>> _adaptors;
	};



	template<typename ImplAdaptor>
	class ModernWDBDefsCreatureDisplayDataset : public DatasetCreatureDisplay {
	public:
		using Adaptor = ImplAdaptor;
		ModernWDBDefsCreatureDisplayDataset(GameFileSystem* fs, const WDBReader::GameVersion& version) : DatasetCreatureDisplay()
		{
			//TODO to extra lookup not needed once lookup by id is implemented.
			std::unordered_map<uint32_t, WDBR::Database::RuntimeRecord> extras;
			auto extras_schema = make_wbdr_schema("CreatureDisplayInfoExtra.dbd", version);

			{
				auto extras_file = fs->openFile("dbfilesclient/creaturedisplayinfoextra.db2");
					auto db2 = WDBR::Database::makeDB2File(
						extras_schema,
						extras_file->release()
					);

					for (auto& rec : *db2) {
						if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
							auto [id] = extras_schema(rec).get<uint32_t>("ID");
							extras.emplace(id, std::move(rec));
						}
					}
			}


			_info_schema = std::make_shared<WDBReader::Database::RuntimeSchema>(
				make_wbdr_schema("CreatureDisplayInfo.dbd", version)
			);

			{
				auto info_file = fs->openFile("dbfilesclient/creaturedisplayinfo.db2");
				auto db2 = WDBR::Database::makeDB2File(
					*_info_schema,
					info_file->release()
				);

				_adaptors.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						//TODO handle extra
			//			std::unique_ptr<CreatureDisplayExtraRecordAdaptor> extra = nullptr;

			//			if constexpr (!std::is_same_v<T_ExtraAdaptor, void>)
			//			{
			//				if (rec.data.extendedDisplayInfoId > 0) {
			//					auto found = extras.find(rec.data.extendedDisplayInfoId);
			//					if (found != extras.end()) {
			//						extra = std::move(found->second);
			//						extras.erase(found);
			//					}
			//				}
			//			}

						_adaptors.push_back(
							std::make_unique<ImplAdaptor>(_info_schema, std::move(rec))
						);
					}
				}
			}
		}

		const std::vector<CreatureDisplayRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<CreatureDisplayRecordAdaptor*>&>(this->_adaptors);
		}

	protected:
		std::shared_ptr<WDBReader::Database::RuntimeSchema> _info_schema;
		std::vector<std::unique_ptr<ImplAdaptor>> _adaptors;

	};

	template<typename ImplAdaptor>
	class ModernWDBDefsItemDataset : public DatasetItems {
		public:
			using Adaptor = ImplAdaptor;
			ModernWDBDefsItemDataset(CascFileSystem* fs, const WDBReader::GameVersion& version) : DatasetItems()
			{

				auto sparse_schema = std::make_shared<WDBReader::Database::RuntimeSchema>(
					make_wbdr_schema("ItemSparse.dbd", version)
				);
				auto appearance_schema = std::make_shared<WDBReader::Database::RuntimeSchema>(
					make_wbdr_schema("ItemAppearance.dbd", version)
				);
				auto item_schema = std::make_shared<WDBReader::Database::RuntimeSchema>(
					make_wbdr_schema("Item.dbd", version)
				);


				std::unordered_map<uint32_t, std::shared_ptr<WDBReader::Database::RuntimeRecord>> sparse_map;
				std::unordered_map<uint32_t, std::shared_ptr<WDBReader::Database::RuntimeRecord>> appearance_map;
				std::unordered_multimap<uint32_t, uint32_t> appearance_modifiers; // item.id => appearance.id

				{
					auto sparse_file = fs->openFile("dbfilesclient/itemsparse.db2");
					auto db2 = WDBR::Database::makeDB2File(
						*sparse_schema,
						sparse_file->release()
					);


					for (auto& rec : *db2) {
						if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
							auto [id] = (*sparse_schema)(rec).get<uint32_t>("ID");
							sparse_map.emplace(id, 
								std::make_shared<WDBReader::Database::RuntimeRecord>(std::move(rec))
							);
						}
					}
				}


				{
					auto appearance_file = fs->openFile("dbfilesclient/itemappearance.db2");
					auto db2 = WDBR::Database::makeDB2File(
						*appearance_schema,
						appearance_file->release()
					);


					for (auto& rec : *db2) {
						if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
							auto [id] = (*appearance_schema)(rec).get<uint32_t>("ID");
							appearance_map.emplace(id,
								std::make_shared<WDBReader::Database::RuntimeRecord>(std::move(rec))
							);
						}
					}
				}


				{
					auto modified_schema = make_wbdr_schema("ItemModifiedAppearance.dbd", version);
					auto modifiers_file = fs->openFile("dbfilesclient/itemmodifiedappearance.db2");
					auto db2 = WDBR::Database::makeDB2File(
						modified_schema,
						modifiers_file->release()
					);

					for (auto& rec : *db2) {
						if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
							auto [item_id, appearance_id] = modified_schema(rec).get<uint32_t, uint32_t>("ItemID", "ItemAppearanceID");
							appearance_modifiers.emplace(item_id, appearance_id);
						}
					}
				}

				{
					auto items_file = fs->openFile("dbfilesclient/item.db2");
					auto db2 = WDBR::Database::makeDB2File(
						*item_schema,
						items_file->release()
					);

					_adaptors.reserve(db2->size());

					for (auto& rec : *db2) {
						if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
							const auto [id] = (*item_schema)(rec).get<uint32_t>("ID");

							auto sparse_found = sparse_map.find(id);
							if (sparse_found == sparse_map.end()) {
								continue;
							}


							auto appearance_range = appearance_modifiers.equal_range(id);
							if (appearance_range.first == appearance_range.second) {
								continue;
							}

							//TODO handle multiple appearances
							auto appearance_found = appearance_map.find(appearance_range.first->second);

							if (appearance_found == appearance_map.end()) {
								continue;
							}


							_adaptors.push_back(
								std::make_unique<ImplAdaptor>(
									item_schema, 
									std::move(rec),
									std::make_pair(sparse_schema, sparse_found->second),
									std::make_pair(appearance_schema, appearance_found->second)
								)
							);


						}
					}
				}
			}


			const std::vector<ItemRecordAdaptor*>& all() const override {
				return reinterpret_cast<const std::vector<ItemRecordAdaptor*>&>(this->_adaptors);
			}

		protected:
			std::vector<std::unique_ptr<ImplAdaptor>> _adaptors;

	};


	template<typename ImplAdaptor>
	class ModernWDBDefsItemDisplayInfoDataset : public DatasetItemDisplay {
	public:
		using Adaptor = ImplAdaptor;
		ModernWDBDefsItemDisplayInfoDataset(CascFileSystem* fs, const WDBReader::GameVersion& version, const IFileDataGameDatabase* fdDB) : DatasetItemDisplay()
		{

			auto schema = std::make_shared<WDBReader::Database::RuntimeSchema>(
				make_wbdr_schema("ItemDisplayInfo.dbd", version)
			);
			auto materials_schema = std::make_shared<WDBReader::Database::RuntimeSchema>(
				make_wbdr_schema("ItemDisplayInfoMaterialRes.dbd", version)
			);
			std::unordered_multimap<uint32_t, WDBReader::Database::RuntimeRecord> materials_map;

			{
				auto materials_file = fs->openFile("dbfilesclient/itemdisplayinfomaterialres.db2");
				auto db2 = WDBR::Database::makeDB2File(
					*materials_schema,
					materials_file->release()
				);

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						auto [id] = (*materials_schema)(rec).get<uint32_t>("ItemDisplayInfoID");
						materials_map.emplace(id, std::move(rec));
					}
				}
			}

			{
				auto display_file = fs->openFile("dbfilesclient/itemdisplayinfo.db2");
				auto db2 = WDBR::Database::makeDB2File(
					*schema,
					display_file->release()
				);

				_adaptors.reserve(db2->size());

				for (auto& rec : *db2) {
					if (rec.encryptionState != WDBR::Database::RecordEncryption::ENCRYPTED) {
						const auto [id] = (*schema)(rec).get<uint32_t>("ID");

						std::vector<WDBReader::Database::RuntimeRecord> mats;
						auto range = materials_map.equal_range(id);
						for (auto it = range.first; it != range.second; ++it) {
							mats.push_back(std::move(it->second));
						}
						materials_map.erase(id);

						_adaptors.push_back(
							std::make_unique<ImplAdaptor>(schema, std::move(rec), materials_schema, std::move(mats), fdDB)
						);
					}
				}
			}
		}


		const std::vector<ItemDisplayRecordAdaptor*>& all() const override {
			return reinterpret_cast<const std::vector<ItemDisplayRecordAdaptor*>&>(this->_adaptors);
		}

	protected:
		std::vector<std::unique_ptr<ImplAdaptor>> _adaptors;
	};
}