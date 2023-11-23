#pragma once
#include "ModelSupport.h"
#include "../filesystem/GameFileSystem.h"
#include "../database/BFARecordDefinitions.h"
#include "../database/DB2File.h"
#include <unordered_set>

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
					gameFS->closeFile(file);
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
	class ModernTabardCustomizationProvider : public TabardCustomizationProvider {
	public:
		ModernTabardCustomizationProvider(GameFileSystem* fs) : TabardCustomizationProvider() {
			gameFS = fs;

			backgroundDB = std::make_unique<DB2File<BFADB2GuildTabardBackground>>("dbfilesclient/guildtabardbackground.db2");
			borderDB = std::make_unique<DB2File<BFADB2GuildTabardBorder>>("dbfilesclient/guildtabardborder.db2");
			emblemDB = std::make_unique<DB2File<BFADB2GuildTabardEmblem>>("dbfilesclient/guildtabardemblem.db2");

			backgroundDB->open((CascFileSystem*)gameFS);
			borderDB->open((CascFileSystem*)gameFS);
			emblemDB->open((CascFileSystem*)gameFS);

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
					std::unordered_set<uint32_t> background_unqiue;
					foreachRecord(backgroundDB.get(), [&background_unqiue](const BFADB2GuildTabardBackground& bg) {
						background_unqiue.insert(bg.data.color);
					});
					maxOptions.background = background_unqiue.size();
				}

				{
					std::unordered_set<uint32_t> border_unique;
					std::unordered_set<uint32_t> border_color_unique;
					foreachRecord(borderDB.get(), [&border_unique, &border_color_unique](const BFADB2GuildTabardBorder& border) {
						border_unique.insert(border.data.borderId);
						if (border.data.borderId == 1) {
							border_color_unique.insert(border.data.color);
						}
					});
					maxOptions.border = border_unique.size();
					maxOptions.borderColor = border_color_unique.size();
				}

				{
					std::unordered_set<uint32_t> emblem_unique;
					std::unordered_set<uint32_t> emblem_color_unique;
					foreachRecord(emblemDB.get(), [&emblem_unique, &emblem_color_unique](const BFADB2GuildTabardEmblem& emblem) {
						emblem_unique.insert(emblem.data.emblemId);
						if (emblem.data.emblemId == 1) {
							emblem_color_unique.insert(emblem.data.color);
						}
					});
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
				return findFileDataId(backgroundDB.get(), [&](const BFADB2GuildTabardBackground& bg) {
					return bg.data.component == (uint32_t)region && bg.data.tier == tier && bg.data.color == options.background;
				});
			};

			custom.texturesUpperChest[0] = find_bg(CharacterRegion::TORSO_UPPER);	//bg
			custom.texturesLowerChest[0] = find_bg(CharacterRegion::TORSO_LOWER);	//bg

			auto find_icon = [&](CharacterRegion region) {
				return findFileDataId(emblemDB.get(), [&](const BFADB2GuildTabardEmblem& bg) {
					return bg.data.component == (uint32_t)region && bg.data.emblemId == options.icon && bg.data.color == options.iconColor;
				});
			};

			custom.texturesUpperChest[1] = find_icon(CharacterRegion::TORSO_UPPER); //icon
			custom.texturesLowerChest[1] = find_icon(CharacterRegion::TORSO_LOWER); //icon

			auto find_border = [&](CharacterRegion region) {
				return findFileDataId(borderDB.get(), [&](const BFADB2GuildTabardBorder& bg) {
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

		std::unique_ptr<DB2File<BFADB2GuildTabardBackground>> backgroundDB;
		std::unique_ptr<DB2File<BFADB2GuildTabardBorder>> borderDB;
		std::unique_ptr<DB2File<BFADB2GuildTabardEmblem>> emblemDB;

		template<typename T, typename F> 
		void foreachRecord(T* db, F fn) {
			const auto& sections = db->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {
				std::for_each(it->records.cbegin(), it->records.cend(), fn);
			}
		}

		template<typename T, typename P>
		uint32_t findFileDataId(T* db, P pred) {
			const auto& sections = db->getSections();
			for (auto it = sections.begin(); it != sections.end(); ++it) {
				auto result = std::find_if(it->records.cbegin(), it->records.cend(), pred);
				if (result != it->records.cend()) {
					return result->data.fileDataId;
				}
			}
			return 0;
		}

	};


}