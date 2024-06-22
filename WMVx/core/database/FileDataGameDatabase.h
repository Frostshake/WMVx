#pragma once

#include <memory>
#include <array>
#include <span>
#include "../filesystem/GameFileUri.h"
#include "../filesystem/CascFileSystem.h"
#include "../database/GameDatasetAdaptors.h"
#include <WDBReader/Database/DB2File.hpp>

namespace core {

	class CascFileSystem;

	class IFileDataGameDatabase {
	public:
		IFileDataGameDatabase() = default;
		IFileDataGameDatabase(IFileDataGameDatabase&&) = default;
		virtual ~IFileDataGameDatabase() = default;

		virtual GameFileUri::id_t findByMaterialResId(uint32_t id, int8_t index, const std::optional<CharacterRelationSearchContext>& search) const = 0;

		template<uint32_t size>
		constexpr std::array<GameFileUri::id_t, size> findByMaterialResIdFixed(std::array<uint32_t, size> ids, const std::optional<CharacterRelationSearchContext>& search) const 
		{
			std::array<GameFileUri::id_t, size> result({ 0 });
			this->findByMaterialResIdFixed(std::span<uint32_t>(ids), std::span<GameFileUri::id_t>(result), search);
			return result;
		}

		virtual GameFileUri::id_t findByModelResId(uint32_t id, int8_t index, const std::optional<CharacterRelationSearchContext>& search) const = 0;

		template<uint32_t size>
		constexpr std::array<GameFileUri::id_t, size> findByModelResIdFixed(std::array<uint32_t, size> ids, const std::optional<CharacterRelationSearchContext>& search) const 
		{
			std::array<GameFileUri::id_t, size> result({ 0 });
			this->findByModelResIdFixed(std::span<uint32_t>(ids), std::span<GameFileUri::id_t>(result), search);
			return result;
		}


	protected:
		virtual void findByMaterialResIdFixed(std::span<uint32_t> ids, std::span<GameFileUri::id_t> dest, const std::optional<CharacterRelationSearchContext>& search) const = 0;
		virtual void findByModelResIdFixed(std::span<uint32_t> ids, std::span<GameFileUri::id_t> dest, const std::optional<CharacterRelationSearchContext>& search) const = 0;
	};

	namespace WDBR = WDBReader;

	template<class ModelFileDataRecord, class TextureFileDataRecord, class ComponentModelFileDataRecord, class ComponentTextureFileDataRecord>
	class FileDataGameDatabase : public IFileDataGameDatabase {
	public:
		FileDataGameDatabase() = default;
		FileDataGameDatabase(FileDataGameDatabase&&) = default;
		virtual ~FileDataGameDatabase() = default;

		inline void loadFileData(CascFileSystem* const fs) 
		{
			auto open_casc_source = [&fs](const auto& name) -> std::unique_ptr<WDBR::Filesystem::CASCFileSource> {
				auto file = fs->openFile(name);
				return static_cast<CascFile*>(file.get())->release();
			};
			
			{
				auto modelFileDataDb = WDBR::Database::makeDB2File<ModelFileDataRecord, WDBR::Filesystem::CASCFileSource>(
					open_casc_source("dbfilesclient/modelfiledata.db2")
				);

				for (auto& rec : *modelFileDataDb) {
					modelFileData.emplace(rec.data.modelResourcesId, rec.data.fileDataId);
				}
			}

			{
				auto textureFileDataDb = WDBR::Database::makeDB2File<TextureFileDataRecord, WDBR::Filesystem::CASCFileSource>(
					open_casc_source("dbfilesclient/texturefiledata.db2")
				);

				for (auto& rec : *textureFileDataDb) {
					textureFileData.emplace(rec.data.materialResourcesId, rec.data.fileDataId);
				}
			}

			//TODO once proper find by id is implemented, these wont need to be memory based.
			{
				auto model_casc_file = open_casc_source("dbfilesclient/componentmodelfiledata.db2");
				auto model_source = std::make_unique<WDBR::Filesystem::MemoryFileSource>(*model_casc_file);
				componentModelFileDataDb = WDBR::Database::makeDB2File<ComponentModelFileDataRecord, WDBR::Filesystem::MemoryFileSource>(std::move(model_source));
			}

			{
				auto texture_casc_file = open_casc_source("dbfilesclient/componenttexturefiledata.db2");
				auto texture_source = std::make_unique<WDBR::Filesystem::MemoryFileSource>(*texture_casc_file);
				componentTextureFileDataDb = WDBR::Database::makeDB2File<ComponentTextureFileDataRecord, WDBR::Filesystem::MemoryFileSource>(std::move(texture_source));
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
						if (std::find(file_ids.begin(), file_ids.end(), it->data.id) != file_ids.end()) {

							if ((it->data.genderIndex == search->gender || search->gender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
								it->data.raceId == search->race) {
								return it->data.id;
							}

							if ((it->data.genderIndex == search->fallbackGender || search->fallbackGender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
								it->data.raceId == search->fallbackRace) {
								fallback_match = it->data.id;
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
						if (std::find(file_ids.begin(), file_ids.end(), it->data.id) != file_ids.end()) {
							if (index < 0 || index == it->data.positionIndex || it->data.positionIndex < 0)
							{
								if ((it->data.genderIndex == search->gender || search->gender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
									it->data.raceId == search->race) {
									return it->data.id;
								}

								if ((it->data.genderIndex == search->fallbackGender || search->fallbackGender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
									it->data.raceId == search->fallbackRace) {
									fallback_match = it->data.id;
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

		std::unique_ptr<WDBR::Database::DataSource<ComponentModelFileDataRecord>> componentModelFileDataDb;
		std::unique_ptr<WDBR::Database::DataSource<ComponentTextureFileDataRecord>> componentTextureFileDataDb;
	};


};