#pragma once

#include <memory>
#include "../filesystem/GameFileUri.h"

namespace core {

	class FileDataGameDatabase {
	public:
		FileDataGameDatabase() = default;
		FileDataGameDatabase(FileDataGameDatabase&&) = default;
		virtual ~FileDataGameDatabase() {}

		//TODO ability to find multiple.

		virtual GameFileUri::id_t findByMaterialResId(uint32_t id) const = 0;

		virtual GameFileUri::id_t findByModelResId(uint32_t id) const = 0;
	};


};