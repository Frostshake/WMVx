#pragma once
#include <string>
#include <QString>

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

		constexpr GameClientVersion(uint8_t major,
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
		struct Environment {
			QString directory;
			QString locale;
			GameClientVersion version;
		};

		struct Profile {
			constexpr Profile(std::string long_name,
				std::string short_name,
				GameClientVersion target_version) :
				longName(long_name),
				shortName(short_name),
				targetVersion(target_version)
			{}

			bool operator==(const Profile&) const = default;

			const std::string longName;
			const std::string shortName;
			const GameClientVersion targetVersion;
		};

		GameClientInfo(Environment e, Profile p) : environment(e), profile(p) {}
		virtual ~GameClientInfo() {};

		static std::optional<Environment> detect(QString path);

		const Environment environment;
		const Profile profile;
	};
};