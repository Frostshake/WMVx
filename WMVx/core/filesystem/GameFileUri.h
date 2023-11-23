#pragma once
#include <QString>
#include <variant>

namespace core {
	//generic file system identifier
	class GameFileUri : public std::variant<uint32_t, QString> {
	public:
		using id_t = uint32_t;	
		using path_t = QString;
		using variant_t = std::variant<uint32_t, QString>;

		using variant_t::variant_t;
		using variant_t::operator=;

		virtual ~GameFileUri() {}

		constexpr bool isId() const {
			return this->index() == 0;
		}

		constexpr bool isPath() const {
			return this->index() == 1;
		}

		id_t getId() const {
			return std::get<id_t>(*this);
		}

		const path_t& getPath() const {
			return std::get<path_t>(*this);
		}

		constexpr bool isEmpty() const {
			return isId() ? getId() == 0 : getPath().isEmpty();
		}

		QString toString() const {
			return isId() ? QString::number(getId()) : getPath();
		}

		inline static path_t removeExtension(path_t src) {
			auto dot_index = src.lastIndexOf('.');
			if (dot_index >= 0) {
				return src.left(dot_index);
			}

			return src;
		}

		inline static path_t replaceExtension(path_t src, QString replacement_extension) {
			return removeExtension(src) + "." + replacement_extension;
		}

		inline static path_t replaceExtension(path_t src, QString expected_extension, QString replacement_extension) {
			if (src.endsWith(expected_extension)) {
				return removeExtension(src) + "." + replacement_extension;
			}

			return src;
		}
	};

	//Sometimes its useful to have both id and path at the same time.
	class GameFileInfo {
	public:
		GameFileUri::id_t id = 0u;
		GameFileUri::path_t path = "";

		inline const QString& toString() const {
			return path;
		}
	};
}