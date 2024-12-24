#pragma once
#include "../filesystem/GameFileUri.h"
#include "../game/GameConstants.h"
#include "../filesystem/GameFileSystem.h"

namespace core {

	/// <summary>
	/// Utility for extracting information about the model form the path name.
	/// </summary>
	class ModelPathInfo {
	public:
		ModelPathInfo() : is_character(false), is_hd_character(false) {
		}

		ModelPathInfo(const GameFileUri::path_t& path, const GameFileSystem* fs) {
			is_character = path.startsWith("char", Qt::CaseInsensitive) || path.startsWith("alternative\\char", Qt::CaseInsensitive);
			is_hd_character = is_character && path.endsWith("_hd.m2");

			if (is_character) {
				const auto parts = path.split(fs->seperator());
				const auto part_count = parts.length();

				if(part_count >= 4) {
					//vanilla - bfa style path
					gender_name = parts[part_count - 2];
					race_name = parts[part_count - 3];
				}
				else if (part_count == 3) {
					//df style path
					
					//df style models are always hd
					is_hd_character = true;
					race_name = parts[part_count - 2];

					const auto& name = parts[part_count - 1];

					if (name.contains("female", Qt::CaseInsensitive)) {
						gender_name = "female";
					}
					else if (name.contains("male", Qt::CaseInsensitive)) {
						gender_name = "male";
					}
					else {
						// some models dont contain a name, but for the purpose of db lookups, its we'll default to male.
						gender_name = "male";
					}
				}
			}
		}

		ModelPathInfo(ModelPathInfo&&) = default;
		ModelPathInfo(const ModelPathInfo&) = default;

		ModelPathInfo& operator= (const ModelPathInfo&) = default;

		inline bool isCharacter() const {
			return is_character;
		}

		inline bool isHdCharacter() const {
			return is_hd_character;
		}

		inline const QString& genderName() const {
			return gender_name;
		}

		inline const QString& raceName() const {
			return race_name;
		}

	protected:
		bool is_character;
		bool is_hd_character;
		QString gender_name;
		QString race_name;
	};

};
