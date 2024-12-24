#pragma once
#include "ModelSupport.h"
#include "../filesystem/GameFileSystem.h"
#include "../filesystem/CascFileSystem.h"
#include <unordered_set>
#include <WDBReader/Database/DB2File.hpp>
#include "../database/WDBDefsGameDatabase.h"

namespace core {

	struct TabardCustomizationOptions {
		uint32_t icon = 0;
		uint32_t iconColor = 0;
		uint32_t border = 0;
		uint32_t borderColor = 0;
		uint32_t background = 0;
	};

	struct TabardCustomization {
		TabardCustomization() {
			texturesUpperChest = { 0ul ,0ul, 0ul };
			texturesLowerChest = { 0ul, 0ul, 0ul };
		}

		std::array<GameFileUri, 3> texturesUpperChest;
		std::array<GameFileUri, 3> texturesLowerChest;
	};

	class TabardCustomizationProvider {
	public:
		TabardCustomizationProvider() = default;
		TabardCustomizationProvider(TabardCustomizationProvider&&) = default;
		virtual ~TabardCustomizationProvider() {}

		/// <summary>
		/// returns an option object containing the count of all the possible options (e.g max_index + 1)
		/// </summary>
		virtual TabardCustomizationOptions getOptionsCount() = 0;

		virtual TabardCustomization getData(const TabardCustomizationOptions& options)  = 0;

		/// <summary>
		///  tiered tabards in the format item_id => tier
		/// </summary>
		virtual std::map<uint32_t, uint32_t> getTieredCustomTabardItemIds() const {
			return {
				{5976, 0},	// regular Guild Tabard
				{69209, 1}, // Illustrious Guild Tabard
				{69210, 2} // Renowned Guild Tabard
			};
		};
	};

	/// <summary>
	/// Legacy file system based tabards
	/// </summary>
	class LegacyTabardCustomisationProvider : public TabardCustomizationProvider {
	public:
		LegacyTabardCustomisationProvider(GameFileSystem* fs) : TabardCustomizationProvider() {
			gameFS = fs;
			loadedMaxOptions = false;
		}
		LegacyTabardCustomisationProvider(LegacyTabardCustomisationProvider&&) = default;
		virtual ~LegacyTabardCustomisationProvider() {}

		virtual TabardCustomizationOptions getOptionsCount() {

			//delayed loading & load once.
			if (!loadedMaxOptions) {
				maxOptions.icon = getMaxOption([](uint32_t i) -> QString {
					return QString("Textures\\GuildEmblems\\Emblem_%1_%2_TU_U.blp")
						.arg(i, 2, 10, QChar('0'))
						.arg("00");
					});
				maxOptions.iconColor = getMaxOption([](uint32_t i) -> QString {
					return QString("Textures\\GuildEmblems\\Emblem_%1_%2_TU_U.blp")
						.arg("00")
						.arg(i, 2, 10, QChar('0'));
					});
				maxOptions.border = getMaxOption([](uint32_t i) -> QString {
					return QString("Textures\\GuildEmblems\\Border_%1_%2_TU_U.blp")
						.arg(i, 2, 10, QChar('0'))
						.arg("00");
					});
				maxOptions.borderColor = getMaxOption([](uint32_t i) -> QString {
					return QString("Textures\\GuildEmblems\\Border_%1_%2_TU_U.blp")
						.arg("00")
						.arg(i, 2, 10, QChar('0'));
					});
				maxOptions.background = getMaxOption([](uint32_t i) -> QString {
					return QString("Textures\\GuildEmblems\\Background_%1_TU_U.blp")
						.arg(i, 2, 10, QChar('0'));
					});

				loadedMaxOptions = true;
			}

			return maxOptions;
		};

		virtual TabardCustomization getData(const TabardCustomizationOptions& options) {
			TabardCustomization custom;

			custom.texturesUpperChest[0] = QString("Textures\\GuildEmblems\\Background_%1_TU_U.blp")
				.arg(options.background, 2, 10, QChar('0'));	//bg
			custom.texturesLowerChest[0] = QString("Textures\\GuildEmblems\\Background_%1_TL_U.blp")
				.arg(options.background, 2, 10, QChar('0'));	//bg

			custom.texturesUpperChest[1] = QString("Textures\\GuildEmblems\\Emblem_%1_%2_TU_U.blp")
				.arg(options.icon, 2, 10, QChar('0'))
				.arg(options.iconColor, 2, 10, QChar('0')); //icon
			custom.texturesLowerChest[1] = QString("Textures\\GuildEmblems\\Emblem_%1_%2_TL_U.blp")
				.arg(options.icon, 2, 10, QChar('0'))
				.arg(options.iconColor, 2, 10, QChar('0')); //icon

			custom.texturesUpperChest[2] = QString("Textures\\GuildEmblems\\Border_%1_%2_TU_U.blp")
				.arg(options.border, 2, 10, QChar('0'))
				.arg(options.borderColor, 2, 10, QChar('0')); //border
			custom.texturesLowerChest[2] = QString("Textures\\GuildEmblems\\Border_%1_%2_TL_U.blp")
				.arg(options.border, 2, 10, QChar('0'))
				.arg(options.borderColor, 2, 10, QChar('0')); //border

			return custom;
		};

	protected:
		GameFileSystem* gameFS;


	private:
		
		bool loadedMaxOptions;
		TabardCustomizationOptions maxOptions;

		uint32_t getMaxOption(std::function<QString(uint32_t)> path)
		{
			uint32_t count = 0;
			bool exists = false;

			do {
				auto file = gameFS->openFile(path(count));
				if (file != nullptr) {
					exists = true;
					count++;
				}
				else {
					exists = false;
				}

			} while (exists);

			return count;
		}

	};

	/// <summary>
	/// Modern DB2 based tabards
	/// </summary>
	/// 
	class ModernTabardCustomizationProvider : public TabardCustomizationProvider {
	public:
		ModernTabardCustomizationProvider(GameFileSystem* fs, const WDBReader::GameVersion& version) : TabardCustomizationProvider() {
			gameFS = fs;

			auto open_source = [&fs](const auto& name) {
				auto file = fs->openFile(name);
				return file->release();
			};

			_schema_background = make_wbdr_schema("GuildTabardBackground.dbd", version);
			_schema_border = make_wbdr_schema("GuildTabardBorder.dbd", version);
			_schema_emblem = make_wbdr_schema("GuildTabardEmblem.dbd", version);
			
			_db_background = WDBReader::Database::makeDB2File(
				_schema_background,
				open_source("dbfilesclient/guildtabardbackground.db2")
			);
			_db_border = WDBReader::Database::makeDB2File(
				_schema_border,
				open_source("dbfilesclient/guildtabardborder.db2")
			);
			_db_emblem = WDBReader::Database::makeDB2File(
				_schema_emblem,
				open_source("dbfilesclient/guildtabardemblem.db2")
			);

			loadedMaxOptions = false;
		}

		virtual TabardCustomizationOptions getOptionsCount() {
			
			//delayed loading & load once.
			if (!loadedMaxOptions) {

				maxOptions.icon = 0;
				maxOptions.iconColor = 0;
				maxOptions.border = 0;
				maxOptions.borderColor = 0;
				maxOptions.background = 0;

				{
					std::unordered_set<uint32_t> background_unique;
					for (auto& rec : *_db_background) {
						if (rec.encryptionState != WDBReader::Database::RecordEncryption::ENCRYPTED) {
							const auto [color] = _schema_background(rec).get<uint32_t>("Color");
							background_unique.insert(color);
						}
					}
					maxOptions.background = background_unique.size();
				}

				{
					std::unordered_set<uint32_t> border_unique;
					std::unordered_set<uint32_t> border_color_unique;
					for (auto& rec : *_db_border) {
						if (rec.encryptionState != WDBReader::Database::RecordEncryption::ENCRYPTED) {
							const auto [color, border] = _schema_border(rec).get<uint32_t, uint32_t>("Color", "BorderID");
							border_unique.insert(border);
							if (border == 1) {
								border_color_unique.insert(color);
							}
						}
					}
					maxOptions.border = border_unique.size();
					maxOptions.borderColor = border_color_unique.size();
				}

				{
					std::unordered_set<uint32_t> emblem_unique;
					std::unordered_set<uint32_t> emblem_color_unique;
					for (auto& rec : *_db_emblem)
					{
						if (rec.encryptionState != WDBReader::Database::RecordEncryption::ENCRYPTED) {
							const auto [color, emblem] = _schema_emblem(rec).get<uint32_t, uint32_t>("Color", "EmblemID");
							emblem_unique.insert(emblem);
							if (emblem == 1) {
								emblem_color_unique.insert(color);
							}
						}
					}
					maxOptions.icon = emblem_unique.size();
					maxOptions.iconColor = emblem_color_unique.size();
				}

				loadedMaxOptions = true;
			}

			return maxOptions;
		};

		virtual TabardCustomization getData(const TabardCustomizationOptions& options) {
			TabardCustomization custom;

			const uint32_t tier = 0;	//TODO USE TIER

			auto find_bg = [&](CharacterRegion region) {
				return findFileDataId(_db_background.get(), _schema_background, [&](const WDBReader::Database::RuntimeRecord& rec) {
					const auto [color, component, tier] = _schema_background(rec).get<uint32_t, uint32_t, uint32_t>("Color", "Component", "Tier");
					return component == (uint32_t)region && tier == tier && color == options.background;
				});
			};

			custom.texturesUpperChest[0] = find_bg(CharacterRegion::TORSO_UPPER);	//bg
			custom.texturesLowerChest[0] = find_bg(CharacterRegion::TORSO_LOWER);	//bg

			auto find_icon = [&](CharacterRegion region) {
				return findFileDataId(_db_emblem.get(), _schema_emblem, [&](const WDBReader::Database::RuntimeRecord& rec) {
					const auto [color, component, emblem] = _schema_emblem(rec).get<uint32_t, uint32_t, uint32_t>("Color", "Component", "EmblemID");
					return component == (uint32_t)region && emblem == options.icon && color == options.iconColor;
				});
			};

			custom.texturesUpperChest[1] = find_icon(CharacterRegion::TORSO_UPPER); //icon
			custom.texturesLowerChest[1] = find_icon(CharacterRegion::TORSO_LOWER); //icon

			auto find_border = [&](CharacterRegion region) {
				return findFileDataId(_db_border.get(), _schema_border, [&](const WDBReader::Database::RuntimeRecord& rec) {
					const auto [color, component, border] = _schema_border(rec).get<uint32_t, uint32_t, uint32_t>("Color", "Component", "BorderID");
					return component == (uint32_t)region && border == options.border && color == options.borderColor;
				});
			};

			custom.texturesUpperChest[2] = find_border(CharacterRegion::TORSO_UPPER); //border
			custom.texturesLowerChest[2] = find_border(CharacterRegion::TORSO_LOWER); //border

			return custom;
		};

	protected:
		GameFileSystem* gameFS;

	private:

		bool loadedMaxOptions;
		TabardCustomizationOptions maxOptions;

		WDBReader::Database::RuntimeSchema _schema_background;
		WDBReader::Database::RuntimeSchema _schema_border;
		WDBReader::Database::RuntimeSchema _schema_emblem;

		std::unique_ptr<WDBReader::Database::DataSource<WDBReader::Database::RuntimeRecord>> _db_background;
		std::unique_ptr<WDBReader::Database::DataSource<WDBReader::Database::RuntimeRecord>> _db_border;
		std::unique_ptr<WDBReader::Database::DataSource<WDBReader::Database::RuntimeRecord>> _db_emblem;


		template<typename T, typename S, typename P>
		uint32_t findFileDataId(T* db, const S& schema, P pred) {
			for (auto& rec : *db) {
				if (rec.encryptionState != WDBReader::Database::RecordEncryption::ENCRYPTED) {
					if (pred(rec)) {
						auto [file_id] = schema(rec).get<uint32_t>("FileDataID");
						return file_id;
					}
				}
			}
			return 0;
		}

	};


}