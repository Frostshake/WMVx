#pragma once
#include <string>
#include <QString>
#include <WDBReader/Detection.hpp>
#include <WDBReader/Utility.hpp>

namespace core {

	class GameClientInfo {
	public:
		struct Environment {
			QString directory;
			QString locale;
			WDBReader::Utility::GameVersion version;
		};

		struct Profile {
			constexpr Profile(std::string long_name,
				std::string short_name,
				WDBReader::Utility::GameVersion target_version) :
				longName(long_name),
				shortName(short_name),
				targetVersion(target_version)
			{}

			bool operator==(const Profile&) const = default;

			const std::string longName;
			const std::string shortName;
			const WDBReader::Utility::GameVersion targetVersion;
		};

		GameClientInfo(Environment e, Profile p) : environment(e), profile(p) {}

		const Environment environment;
		const Profile profile;
	};
};