#pragma once
#include <string>
#include <QString>
#include <WDBReader/Detection.hpp>
#include <WDBReader/Utility.hpp>

namespace core {

	enum GameGeneration : uint16_t {
		VANILLA = (1 << 8),
		THE_BURNING_CRUSADE = (2 << 8),
		WRATH_OF_THE_LICH_KING = (3 << 8),
		CATACLYSM = (4 << 8),
		MISTS_OF_PANDARIA = (5 << 8),
		WARLORDS_OF_DRAENOR = (6 << 8),
		LEGION = (7 << 8),
		BATTLE_FOR_AZEROTH = (8 << 8),
		SHADOWLANDS = (9 << 8),
		DRAGONFLIGHT = (10 << 8),
		THE_WAR_WITHIN = (11 << 8)
	};

	class GameClientInfo {
	public:
		struct Environment {
			QString directory;
			QString locale;
			WDBReader::GameVersion version;
		};

		struct Profile {
			constexpr Profile(std::string long_name,
				std::string short_name,
				WDBReader::GameVersion target_version) :
				longName(long_name),
				shortName(short_name),
				targetVersion(target_version)
			{}

			bool operator==(const Profile&) const = default;

			const std::string longName;
			const std::string shortName;
			const WDBReader::GameVersion targetVersion;
		};

		GameClientInfo(Environment e, Profile p) : environment(e), profile(p) {}

		const Environment environment;
		const Profile profile;
	};
};