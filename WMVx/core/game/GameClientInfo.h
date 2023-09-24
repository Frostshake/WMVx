#pragma once
#include <QString>
#include <memory>
#include <optional>
#include "GameClientAdaptor.h"

namespace core {

	struct GameClientVersion {
		uint8_t major;
		uint8_t minor;
		uint8_t patch;
		uint32_t build;

		GameClientVersion() :
			major(0),
			minor(0),
			patch(0),
			build(0)
		{}

		GameClientVersion(uint8_t major,
			uint8_t minor,
			uint8_t patch,
			uint32_t build) :
			major(major),
			minor(minor),
			patch(patch),
			build(build)
		{}

		operator QString() const {
			return QString("%1.%2.%3 (%4)").arg(major).arg(minor).arg(patch).arg(build);
		}

		auto operator<=>(const GameClientVersion&) const = default;

		static std::optional<GameClientVersion> fromString(QString str) {
			
			auto parts = str.split(".");

			if (parts.length() == 4) {
				return GameClientVersion(
					parts[0].toInt(), 
					parts[1].toInt(), 
					parts[2].toInt(), 
					parts[3].toInt()
				);
			}
			
			return std::nullopt;
		}
	};

	class GameClientInfo {
	public:
		GameClientInfo() {}

		static std::optional<GameClientInfo> detect(QString path);
		std::unique_ptr<GameClientAdaptor> adaptor() const;

		QString directory;
		QString locale;
		GameClientVersion version;
	};
};