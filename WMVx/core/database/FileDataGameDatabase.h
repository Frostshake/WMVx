#pragma once

#include <memory>
#include <array>
#include <span>
#include "../filesystem/GameFileUri.h"
#include "../database/DB2File.h"

namespace core {

	class CascFileSystem;

	class IFileDataGameDatabase {
	public:
		IFileDataGameDatabase() = default;
		IFileDataGameDatabase(IFileDataGameDatabase&&) = default;
		virtual ~IFileDataGameDatabase() {}

		//TODO 'in' style query

		virtual GameFileUri::id_t findByMaterialResId(uint32_t id) const = 0;

		template<uint32_t size>
		constexpr std::array<GameFileUri::id_t, size> findByMaterialResIdFixed(uint32_t id) const {
			std::array<GameFileUri::id_t, size> result({ 0 });
			this->findByMaterialResIdFixed(id, std::span<GameFileUri::id_t>(result));
			return result;
		}


		virtual GameFileUri::id_t findByModelResId(uint32_t id) const = 0;

		template<uint32_t size>
		constexpr std::array<GameFileUri::id_t, size> findByModelResIdFixed(uint32_t id) const {
			std::array<GameFileUri::id_t, size> result({ 0 });
			this->findByModelResIdFixed(id, std::span<GameFileUri::id_t>(result));
			return result;
		}

	protected:
		virtual void findByMaterialResIdFixed(uint32_t id, std::span<GameFileUri::id_t> dest) const = 0;
		virtual void findByModelResIdFixed(uint32_t id, std::span<GameFileUri::id_t> dest) const = 0;

	};


	template<class ModelFileDataRecord, class TextureFileDataRecord>
	class FileDataGameDatabase : public IFileDataGameDatabase {
	public:
		FileDataGameDatabase() = default;
		FileDataGameDatabase(FileDataGameDatabase&&) = default;
		virtual ~FileDataGameDatabase() {}

		inline void loadFileData(CascFileSystem* const fs) {
			modelFileDataDB = std::make_unique<DB2File<ModelFileDataRecord>>("dbfilesclient/modelfiledata.db2");
			modelFileDataDB->open(fs);

			textureFileDataDB = std::make_unique<DB2File<TextureFileDataRecord>>("dbfilesclient/texturefiledata.db2");
			textureFileDataDB->open(fs);
		}

		virtual GameFileUri::id_t findByMaterialResId(uint32_t id) const {
			for (auto it = textureFileDataDB->cbegin(); it != textureFileDataDB->cend(); ++it) {
				if (it->data.materialResourcesId == id) {
					return it->data.fileDataId;
				}
			}

			return 0u;
		}

		virtual void findByMaterialResIdFixed(uint32_t id, std::span<GameFileUri::id_t> dest) const {
			int32_t index = 0;
			for (auto it = textureFileDataDB->cbegin(); it != textureFileDataDB->cend(); ++it) {
				if (it->data.materialResourcesId == id) {
					dest[index] = it->data.fileDataId;
					if (++index >= dest.size()) {
						return;
					}
				}
			}
		}

		virtual GameFileUri::id_t findByModelResId(uint32_t id) const {
			for (auto it = modelFileDataDB->cbegin(); it != modelFileDataDB->cend(); ++it) {
				if (it->data.modelResourcesId == id) {
					return it->data.fileDataId;
				}
			}
			
			return 0u;
		}

		virtual void findByModelResIdFixed(uint32_t id, std::span<GameFileUri::id_t> dest) const {
			int32_t index = 0;
			for (auto it = modelFileDataDB->cbegin(); it != modelFileDataDB->cend(); ++it) {
				if (it->data.modelResourcesId == id) {
					dest[index] = it->data.fileDataId;
					if (++index >= dest.size()) {
						return;
					}
				}
				
			}

		}

	protected:
		std::unique_ptr<DB2File<ModelFileDataRecord>> modelFileDataDB;
		std::unique_ptr<DB2File<TextureFileDataRecord>> textureFileDataDB;
	};


};