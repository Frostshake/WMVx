#pragma once

#include <memory>
#include <array>
#include <span>
#include "../filesystem/GameFileUri.h"
#include "../database/DB2File.h"
#include "../database/GameDatasetAdaptors.h"

namespace core {

	class CascFileSystem;

	class IFileDataGameDatabase {
	public:
		IFileDataGameDatabase() = default;
		IFileDataGameDatabase(IFileDataGameDatabase&&) = default;
		virtual ~IFileDataGameDatabase() {}

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


	template<class ModelFileDataRecord, class TextureFileDataRecord, class ComponentModelFileDataRecord, class ComponentTextureFileDataRecord>
	class FileDataGameDatabase : public IFileDataGameDatabase {
	public:

		FileDataGameDatabase() = default;
		FileDataGameDatabase(FileDataGameDatabase&&) = default;
		virtual ~FileDataGameDatabase() {}

		inline void loadFileData(CascFileSystem* const fs) 
		{
			auto modelFileDataDB = std::make_unique<DB2File<ModelFileDataRecord>>("dbfilesclient/modelfiledata.db2");
			modelFileDataDB->open(fs);

			for (auto it = modelFileDataDB->cbegin(); it != modelFileDataDB->cend(); ++it) {
				modelFileData.emplace(it->data.modelResourcesId, it->data.fileDataId);
			}

			auto textureFileDataDB = std::make_unique<DB2File<TextureFileDataRecord>>("dbfilesclient/texturefiledata.db2");
			textureFileDataDB->open(fs);

			for (auto it = textureFileDataDB->cbegin(); it != textureFileDataDB->cend(); ++it) {
				textureFileData.emplace(it->data.materialResourcesId, it->data.fileDataId);
			}

			componentModelFileDataDB = std::make_unique<DB2File<ComponentModelFileDataRecord>>("dbfilesclient/componentmodelfiledata.db2");
			componentModelFileDataDB->open(fs);

			componentTextureFileDataDB = std::make_unique<DB2File<ComponentTextureFileDataRecord>>("dbfilesclient/componenttexturefiledata.db2");
			componentTextureFileDataDB->open(fs);
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

					for (auto it = componentTextureFileDataDB->cbegin(); it != componentTextureFileDataDB->cend(); ++it) {
						if (std::find(file_ids.begin(), file_ids.end(), it->data.id) != file_ids.end()) {

							if ((it->data.genderIndex == search->gender || search->gender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
								it->data.raceID == search->race) {
								return it->data.id;
							}

							if ((it->data.genderIndex == search->fallbackGender || search->fallbackGender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
								it->data.raceID == search->fallbackRace) {
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

					for (auto it = componentModelFileDataDB->cbegin(); it != componentModelFileDataDB->cend(); ++it) {
						if (std::find(file_ids.begin(), file_ids.end(), it->data.id) != file_ids.end()) {
							if (index < 0 || index == it->data.positionIndex || it->data.positionIndex < 0)
							{
								if ((it->data.genderIndex == search->gender || search->gender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
									it->data.raceID == search->race) {
									return it->data.id;
								}

								if ((it->data.genderIndex == search->fallbackGender || search->fallbackGender == CharacterRelationSearchContext::MODERN_GENDER_IGNORE) &&
									it->data.raceID == search->fallbackRace) {
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

		//TODO optimise.
		std::unique_ptr<DB2File<ComponentModelFileDataRecord>> componentModelFileDataDB;
		std::unique_ptr<DB2File<ComponentTextureFileDataRecord>> componentTextureFileDataDB;
	};


};