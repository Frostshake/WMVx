#pragma once
#include "ModelSupport.h"
#include "../filesystem/GameFileSystem.h"
#include "../filesystem/CascFileSystem.h"
#include <unordered_set>
#include <WDBReader/Database/DB2File.hpp>

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
	template<WDBReader::Database::TRecord TabardBackground, WDBReader::Database::TRecord TabardBorder, WDBReader::Database::TRecord TabardEmblem>
	class ModernTabardCustomizationProvider : public TabardCustomizationProvider {
	public:
		ModernTabardCustomizationProvider(GameFileSystem* fs) : TabardCustomizationProvider() {
			gameFS = fs;

			auto open_source = [&fs](const auto& name) {
				auto file = fs->openFile(name);
				return static_cast<CascFile*>(file.get())->release();
			};
			
			backgroundDB = WDBReader::Database::makeDB2File<TabardBackground>(
				open_source("dbfilesclient/guildtabardbackground.db2")
			);
			borderDB = WDBReader::Database::makeDB2File<TabardBorder>(
				open_source("dbfilesclient/guildtabardborder.db2")
			);
			emblemDB = WDBReader::Database::makeDB2File<TabardEmblem>(
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
					for (auto& rec : *backgroundDB) {
						background_unique.insert(rec.data.color);
					}
					maxOptions.background = background_unique.size();
				}

				{
					std::unordered_set<uint32_t> border_unique;
					std::unordered_set<uint32_t> border_color_unique;
					for (auto& rec : *borderDB) {
						border_unique.insert(rec.data.borderId);
						if (rec.data.borderId == 1) {
							border_color_unique.insert(rec.data.color);
						}
					}
					maxOptions.border = border_unique.size();
					maxOptions.borderColor = border_color_unique.size();
				}

				{
					std::unordered_set<uint32_t> emblem_unique;
					std::unordered_set<uint32_t> emblem_color_unique;
					for (auto& rec : *emblemDB)
					{
						emblem_unique.insert(rec.data.emblemId);
						if (rec.data.emblemId == 1) {
							emblem_color_unique.insert(rec.data.color);
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
				return findFileDataId(backgroundDB.get(), [&](const TabardBackground& bg) {
					return bg.data.component == (uint32_t)region && bg.data.tier == tier && bg.data.color == options.background;
				});
			};

			custom.texturesUpperChest[0] = find_bg(CharacterRegion::TORSO_UPPER);	//bg
			custom.texturesLowerChest[0] = find_bg(CharacterRegion::TORSO_LOWER);	//bg

			auto find_icon = [&](CharacterRegion region) {
				return findFileDataId(emblemDB.get(), [&](const TabardEmblem& bg) {
					return bg.data.component == (uint32_t)region && bg.data.emblemId == options.icon && bg.data.color == options.iconColor;
				});
			};

			custom.texturesUpperChest[1] = find_icon(CharacterRegion::TORSO_UPPER); //icon
			custom.texturesLowerChest[1] = find_icon(CharacterRegion::TORSO_LOWER); //icon

			auto find_border = [&](CharacterRegion region) {
				return findFileDataId(borderDB.get(), [&](const TabardBorder& bg) {
					return bg.data.component == (uint32_t)region && bg.data.borderId == options.border && bg.data.color == options.borderColor;
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

		std::unique_ptr<WDBReader::Database::DataSource<TabardBackground>> backgroundDB;
		std::unique_ptr<WDBReader::Database::DataSource<TabardBorder>> borderDB;
		std::unique_ptr<WDBReader::Database::DataSource<TabardEmblem>> emblemDB;


		template<typename T, typename P>
		uint32_t findFileDataId(T* db, P pred) {
			auto result = std::find_if(db->begin(), db->end(), pred);
			if (result != db->end()) {
				return result->data.fileDataId;
			}
			return 0;
		}

	};


}