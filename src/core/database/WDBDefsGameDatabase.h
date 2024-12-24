#pragma once

#include "GameDatabase.h"
#include "FileDataGameDatabase.h"
#include "WDBDefsDatasets.h"
#include <WDBReader/WoWDBDefs.hpp>
#include <fstream>
#include <future>

namespace core {

	class WDBDefsFileDataGameDatabase : public IFileDataGameDatabase {
	public:
		WDBDefsFileDataGameDatabase(WDBReader::GameVersion& ver) : version(ver) {}
		WDBDefsFileDataGameDatabase(WDBDefsFileDataGameDatabase&&) = default;
		virtual ~WDBDefsFileDataGameDatabase() = default;

		inline void loadFileData(CascFileSystem* const fs)
		{
			auto open_casc_source = [&fs](const auto& name) -> std::unique_ptr<WDBR::Filesystem::FileSource> {
				auto file = fs->openFile(name);
				return file->release();
			};

			{
				auto schema = make_wbdr_schema("ModelFileData.dbd", version);
				auto modelFileDataDb = WDBR::Database::makeDB2File(
					schema,
					open_casc_source("dbfilesclient/modelfiledata.db2")
				);

				//TODO confirm fields exist in schema;

				for (auto& rec : *modelFileDataDb) {
					auto accessor = schema(rec);
					auto [model_res_id, file_data_id] = accessor.get<uint32_t, uint32_t>("ModelResourcesID", "FileDataID");
					modelFileData.emplace(model_res_id, file_data_id);
				}
			}

			{
				auto schema = make_wbdr_schema("TextureFileData.dbd", version);
				auto textureFileDataDb = WDBR::Database::makeDB2File(
					schema,
					open_casc_source("dbfilesclient/texturefiledata.db2")
				);

				//TODO confirm fields exist in schema;

				for (auto& rec : *textureFileDataDb) {
					auto accessor = schema(rec);
					auto [mat_res_id, file_data_id] = accessor.get<uint32_t, uint32_t>("MaterialResourcesID", "FileDataID");
					textureFileData.emplace(mat_res_id, file_data_id);
				}
			}

			compModelDataSchema = make_wbdr_schema("ComponentModelFileData.dbd", version);
			compTextureDataSchema = make_wbdr_schema("ComponentTextureFileData.dbd", version);


			//TODO once proper find by id is implemented, these wont need to be memory based.
			{
				auto model_casc_file = open_casc_source("dbfilesclient/componentmodelfiledata.db2");
				auto model_source = std::make_unique<WDBR::Filesystem::MemoryFileSource>(*model_casc_file);
				componentModelFileDataDb = WDBR::Database::makeDB2File(compModelDataSchema, std::move(model_source));
			}

			{
				auto texture_casc_file = open_casc_source("dbfilesclient/componenttexturefiledata.db2");
				auto texture_source = std::make_unique<WDBR::Filesystem::MemoryFileSource>(*texture_casc_file);
				componentTextureFileDataDb = WDBR::Database::makeDB2File(compTextureDataSchema, std::move(texture_source));
			}

			if (componentModelFileDataDb == nullptr || componentTextureFileDataDb == nullptr) {
				throw std::runtime_error("Failed to open file data db2 files.");
			}
		}

		virtual GameFileUri::id_t findByMaterialResId(uint32_t id, int8_t index, const std::optional<CharacterRelationSearchContext>& search) const
		{
			auto range = textureFileData.equal_range(id);

			if (range.first != range.second) {

				std::vector<uint32_t> file_ids;
				for (auto it = range.first; it != range.second; ++it) {
					file_ids.push_back(it->second);
				}

				assert(file_ids.size() > 0);

				if (file_ids.size() > 1 && search.has_value()) {
					auto fallback_match = 0;

					for (auto it = componentTextureFileDataDb->cbegin(); it != componentTextureFileDataDb->cend(); ++it) {
						auto [id, genderIndex, raceId] = compTextureDataSchema(*it).get<uint32_t, uint8_t, uint8_t>("ID", "GenderIndex", "RaceID");
						if (std::find(file_ids.begin(), file_ids.end(), id) != file_ids.end()) {

							if ((genderIndex == search->gender || search->gender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
								raceId == search->race) {
								return id;
							}

							if ((genderIndex == search->fallbackGender || search->fallbackGender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
								raceId == search->fallbackRace) {
								fallback_match = id;
							}
						}
					}

					if (fallback_match > 0) {
						return fallback_match;
					}
				}

				return file_ids[0];
			}

			return 0u;
		}

		virtual GameFileUri::id_t findByModelResId(uint32_t id, int8_t index, const std::optional<CharacterRelationSearchContext>& search) const
		{
			auto range = modelFileData.equal_range(id);

			if (range.first != range.second) {

				std::vector<uint32_t> file_ids;
				for (auto it = range.first; it != range.second; ++it) {
					file_ids.push_back(it->second);
				}

				assert(file_ids.size() > 0);

				if (file_ids.size() > 1 && search.has_value()) {
					auto fallback_match = 0;

					//TODO investigate class usage.

					for (auto it = componentModelFileDataDb->cbegin(); it != componentModelFileDataDb->cend(); ++it) {
						auto [id, genderIndex, raceId, positionIndex] = compModelDataSchema(*it).get<uint32_t, uint8_t, uint8_t, int8_t>("ID", "GenderIndex", "RaceID", "PositionIndex");
						if (std::find(file_ids.begin(), file_ids.end(), id) != file_ids.end()) {
							if (index < 0 || index == positionIndex || positionIndex < 0)
							{
								if ((genderIndex == search->gender || search->gender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
									raceId == search->race) {
									return id;
								}

								if ((genderIndex == search->fallbackGender || search->fallbackGender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
									raceId == search->fallbackRace) {
									fallback_match = id;
								}
							}
						}
					}

					if (fallback_match > 0) {
						return fallback_match;
					}
				}

				return file_ids[0];
			}

			return 0u;
		}

	protected:

		virtual void findByMaterialResIdFixed(std::span<uint32_t> ids, std::span<GameFileUri::id_t> dest, const std::optional<CharacterRelationSearchContext>& search) const
		{
			assert(ids.size() == dest.size());
			auto index = 0;

			for (auto id : ids) {
				dest[index] = findByMaterialResId(id, index, search);
				index++;
			}
		}

		virtual void findByModelResIdFixed(std::span<uint32_t> ids, std::span<GameFileUri::id_t> dest, const std::optional<CharacterRelationSearchContext>& search) const
		{
			assert(ids.size() == dest.size());
			auto index = 0;

			for (auto id : ids) {
				dest[index] = findByModelResId(id, index, search);
				index++;
			}
		}

		//relation -> filedata_id format.
		std::unordered_multimap<uint32_t, uint32_t> modelFileData;
		std::unordered_multimap<uint32_t, uint32_t> textureFileData;

		std::unique_ptr<WDBR::Database::DataSource<WDBR::Database::RuntimeRecord>> componentModelFileDataDb;
		std::unique_ptr<WDBR::Database::DataSource<WDBR::Database::RuntimeRecord>> componentTextureFileDataDb;

		WDBR::Database::RuntimeSchema compModelDataSchema;
		WDBR::Database::RuntimeSchema compTextureDataSchema;

	private:
		WDBReader::GameVersion& version;
	};

	class WDBDefsGameDatabase : public GameDatabase, public WDBDefsFileDataGameDatabase
	{
	public:
		WDBDefsGameDatabase(const WDBReader::GameVersion& ver) : version(ver), GameDatabase(), WDBDefsFileDataGameDatabase(version) {}
		WDBDefsGameDatabase(WDBDefsGameDatabase&&) = default;
		virtual ~WDBDefsGameDatabase() {}

		void load(const GameFileSystem* const fs) override {

			auto* const cascFS = (CascFileSystem*)(fs);

			loadFileData(cascFS);

			auto items_async = std::async(std::launch::async, [&]() {
				itemsDB = std::make_unique<ModernWDBDefsItemDataset<ModernWDBDefsItemRecordAdaptor>>(cascFS, version);
			});

			auto items_display_async = std::async(std::launch::async, [&]() {
				itemDisplayDB = std::make_unique<ModernWDBDefsItemDisplayInfoDataset<ModernWDBDefsItemDisplayInfoRecordAdaptor>>(cascFS, version, this);
			});

			animationDataDB = std::make_unique<ModernWDBDefsAnimationDataDataset<ModernWDBDefsAnimationDataRecordAdaptor>>(cascFS, version, "Support Files\\animation-names.csv");

			auto creatures_async = std::async(std::launch::async, [&]() {
				creatureModelDataDB = std::make_unique<GenericWDBDefsDataset<DatasetCreatureModelData,ModernWDBDefsCreatureModelDataRecordAdaptor>>(
					cascFS, 
					"dbfilesclient/creaturemodeldata.db2",
					version,
					"CreatureModelData.dbd"
				);

				creatureDisplayDB = std::make_unique<ModernWDBDefsCreatureDisplayDataset<ModernWDBDefsCreatureDisplayRecordAdaptor>>(cascFS, version);
			});

			characterRacesDB = std::make_unique<GenericWDBDefsDataset<DatasetCharacterRaces, ModernWDBDefsCharRacesRecordAdaptor>>(
				cascFS, 
				"dbfilesclient/chrraces.db2", 
				version,  
				"ChrRaces.dbd"
			);
			characterSectionsDB = nullptr;	//TODO make conditional.

			//TODO / conditional.
			//characterFacialHairStylesDB = std::make_unique<DFCharacterFacialHairStylesDataset>(cascFS, "dbfilesclient/characterfacialhairstyles.db2");
			//characterHairGeosetsDB = std::make_unique<DFCharHairGeosetsDataset>(cascFS, "dbfilesclient/charhairgeosets.db2");

			characterComponentTexturesDB = std::make_unique<ModernWDBDefsCharacterComponentTextureDataset<ModernWDBDefsCharacterComponentTextureAdaptor>>(cascFS, version);

			//TODO
			itemVisualsDB = nullptr;
			itemVisualEffectsDB = nullptr;
			spellEnchantmentsDB = nullptr;

			npcsDB = std::make_unique<GenericWDBDefsDataset<DatasetNPCs, ModernWDBDefsNPCRecordAdaptor>>(
				cascFS,
				"dbfilesclient/creature.db2",
				version,
				"Creature.dbd"
			);

			creatures_async.wait();
			items_async.wait();
			items_display_async.wait();
		}


	protected:
		WDBReader::GameVersion version;
	};


};