#include "../../stdafx.h"
#include <QDir>
#include <memory>
#include "GameClientInfo.h"
#include "GameClientAdaptor.h"
#include "../filesystem/MPQFileSystem.h"
#include "../filesystem/CascFileSystem.h"
#include "../database/VanillaGameDatabase.h"
#include "../database/WOTLKGameDatabase.h"
#include "../database/BFAGameDatabase.h"
#include "../database/DFGameDatabase.h"
#include "../database/WDBDefsGameDatabase.h"

namespace core {

	std::unique_ptr<GameFileSystem> VanillaGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment) {
		return std::make_unique<MPQFileSystem>(environment.directory + QDir::separator() + "Data", environment.locale);
	}

	std::unique_ptr<GameDatabase> VanillaGameClientAdaptor::database()
	{
		return std::make_unique<VanillaGameDatabase>();
	}

	const ModelSupport VanillaGameClientAdaptor::modelSupport()
	{
		auto mf = &M2Model::make;

		return ModelSupport(
			mf,
			[](GameFileSystem* fs) {
				return std::make_unique<LegacyTabardCustomisationProvider>(fs);
			},
			[](GameFileSystem* fs, GameDatabase* db) {
				return std::make_unique<LegacyCharacterCustomizationProvider>(fs, db);
			},
			[mf](GameFileSystem* fs, GameDatabase* db) {
				return std::make_unique<StandardAttachmentCustomizationProvider>(fs, db, mf);
			}
		);
	}

	const GameClientInfo::Profile VanillaGameClientAdaptor::PROFILE {
			"Vanilla",
			"Vanilla",
			{ 1, 12, 1, 5875 }
	};

	std::unique_ptr<GameFileSystem> WOTLKGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<MPQFileSystem>(environment.directory + QDir::separator() + "Data", environment.locale);
	}

	std::unique_ptr<GameDatabase> WOTLKGameClientAdaptor::database()
	{
		return std::make_unique<WOTLKGameDatabase>(WOTLKGameDatabase());
	}

	const ModelSupport WOTLKGameClientAdaptor::modelSupport()
	{
		auto mf = &M2Model::make;

		return ModelSupport(
			mf,
			[](GameFileSystem* fs) {
				return std::make_unique<LegacyTabardCustomisationProvider>(fs);
			},
			[](GameFileSystem* fs, GameDatabase* db) {
				return std::make_unique<LegacyCharacterCustomizationProvider>(fs, db);
			},
			[mf](GameFileSystem* fs, GameDatabase* db) {
				return std::make_unique<StandardAttachmentCustomizationProvider>(fs, db, mf);
			}
		);
	}

	const GameClientInfo::Profile WOTLKGameClientAdaptor::PROFILE {
		"Wrath of the Lich King",
		"WotLK",
		{ 3,  3, 5, 12340 }
	};

	std::unique_ptr<GameFileSystem> BFAGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<CascFileSystem>(environment.directory, environment.locale, "Support Files\\bfa\\listfile.csv"); //intentionally not appending 'Data'
	}

	std::unique_ptr<GameDatabase> BFAGameClientAdaptor::database()
	{
		return std::make_unique<BFAGameDatabase>();
	}

	const ModelSupport BFAGameClientAdaptor::modelSupport()
	{
		auto mf = &M2Model::make;
		auto ver = getClientInfo().environment.version;

		return ModelSupport(
			mf,
			[ver](GameFileSystem* fs) {
				return std::make_unique<ModernTabardCustomizationProvider>(fs, ver);
			},
			[](GameFileSystem* fs, GameDatabase* db) {
				return std::make_unique<LegacyCharacterCustomizationProvider>(fs, db);
			},
			[mf](GameFileSystem* fs, GameDatabase* db) {
				return std::make_unique<StandardAttachmentCustomizationProvider>(fs, db, mf);
			}
		);
	}

	const GameClientInfo::Profile BFAGameClientAdaptor::PROFILE {
		"Battle for Azeroth",
		"BFA",
		{ 8, 3, 7, 35435 }
	};

	std::unique_ptr<GameFileSystem> DFGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<CascFileSystem>(environment.directory, environment.locale, "Support Files\\df\\listfile.csv"); //intentionally not appending 'Data'
	}

	std::unique_ptr<GameDatabase> DFGameClientAdaptor::database()
	{
		return std::make_unique<DFGameDatabase>();
	}

	const ModelSupport DFGameClientAdaptor::modelSupport()
	{
		auto mf = &M2Model::make;
		auto ver = getClientInfo().environment.version;

		return ModelSupport(
			mf,
			[ver](GameFileSystem* fs) {
				return std::make_unique<ModernTabardCustomizationProvider>(fs, ver);
			},
			[ver](GameFileSystem* fs, GameDatabase* db) {
				auto tmp = std::make_unique<ModernCharacterCustomizationProvider>(fs, db, ver);
				tmp->setCharacterEyeGlowHandler(CharacterEyeGlowCustomization::geosetBasedHandler);
				return tmp;
			},
			[mf](GameFileSystem* fs, GameDatabase* db) {
				return std::make_unique<MergedAwareAttachmentCustomizationProvider>(fs, db, mf);
			}
		);
	}

	const GameClientInfo::Profile DFGameClientAdaptor::PROFILE {
		"Dragonflight",
		"DF",
		{ 10, 2, 7, 55142 }
	};

	std::unique_ptr<GameFileSystem> TWWGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<CascFileSystem>(environment.directory, environment.locale, "Support Files\\tww\\listfile.csv"); //intentionally not appending 'Data'
	}

	std::unique_ptr<GameDatabase> TWWGameClientAdaptor::database()
	{
		return std::make_unique<WDBDefsGameDatabase>(getClientInfo().environment.version);
	}

	const ModelSupport TWWGameClientAdaptor::modelSupport()
	{
		auto mf = &M2Model::make;
		auto ver = getClientInfo().environment.version;

		return ModelSupport(
			mf,
			[ver](GameFileSystem* fs) {
				return std::make_unique<ModernTabardCustomizationProvider>(fs, ver);
			},
			[ver](GameFileSystem* fs, GameDatabase* db) {
				auto tmp = std::make_unique<ModernCharacterCustomizationProvider>(fs, db, ver);
				tmp->setCharacterEyeGlowHandler(CharacterEyeGlowCustomization::geosetBasedHandler);
				return tmp;
			},
			[mf](GameFileSystem* fs, GameDatabase* db) {
				return std::make_unique<MergedAwareAttachmentCustomizationProvider>(fs, db, mf);
			}
		);
	}

	const GameClientInfo::Profile TWWGameClientAdaptor::PROFILE{
		"The War Within",
		"TWW",
		{ 11, 0, 0, 56008 }
	};

	std::unique_ptr<GameClientAdaptor> makeGameClientAdaptor(const GameClientInfo& info) {
		
		if (VanillaGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<VanillaGameClientAdaptor>(info);
		}

		if (WOTLKGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<WOTLKGameClientAdaptor>(info);
		}

		if (BFAGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<BFAGameClientAdaptor>(info);
		}

		if (DFGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<DFGameClientAdaptor>(info);
		}

		if (TWWGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<TWWGameClientAdaptor>(info);
		}

		return nullptr;
	}
}