#include "../../stdafx.h"
#include <QDir>
#include <memory>
#include "GameClientInfo.h"
#include "GameClientAdaptor.h"
#include "../filesystem/MPQFileSystem.h"
#include "../filesystem/CascFileSystem.h"
#include "../database/VanillaGameDatabase.h"
#include "../database/TBCGameDatabase.h"
#include "../database/WOTLKGameDatabase.h"
#include "../database/CataGameDatabase.h"
#include "../database/BFAGameDatabase.h"
#include "../database/WDBDefsGameDatabase.h"
#include "../modeling/Geoset.h"

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
			"1.12.1.5875",
			{ 1, 12, 1, 5875 }
	};


	std::unique_ptr<GameFileSystem> TBCGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<MPQFileSystem>(environment.directory + QDir::separator() + "Data", environment.locale);
	}

	std::unique_ptr<GameDatabase> TBCGameClientAdaptor::database()
	{
		return std::make_unique<TBCGameDatabase>();
	}

	const ModelSupport TBCGameClientAdaptor::modelSupport()
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

	const GameClientInfo::Profile TBCGameClientAdaptor::PROFILE{
		"The Burning Crusade",
		"TBC",
		"2.4.3.8606",
		{ 2,  4, 3, 8606 }
	};

	std::unique_ptr<GameFileSystem> WOTLKGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<MPQFileSystem>(environment.directory + QDir::separator() + "Data", environment.locale);
	}

	std::unique_ptr<GameDatabase> WOTLKGameClientAdaptor::database()
	{
		return std::make_unique<WOTLKGameDatabase>();
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
		"3.3.5.12340",
		{ 3,  3, 5, 12340 }
	};

	std::unique_ptr<GameFileSystem> CataGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<MPQFileSystem>(environment.directory + QDir::separator() + "Data", environment.locale);
	}

	std::unique_ptr<GameDatabase> CataGameClientAdaptor::database()
	{
		return std::make_unique<CataGameDatabase>();
	}

	const ModelSupport CataGameClientAdaptor::modelSupport()
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

	const GameClientInfo::Profile CataGameClientAdaptor::PROFILE{
		"Cataclysm",
		"Cata",
		"4.3.4.15595",
		{ 4,  3, 4, 15595 }
	};

	std::unique_ptr<GameFileSystem> BFAGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<CascFileSystem>(environment.directory, environment.locale, environment.product, "Support Files\\bfa\\listfile.csv"); //intentionally not appending 'Data'
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
		"8.3.7.35435",
		{ 8, 3, 7, 35435 }
	};

	std::unique_ptr<GameFileSystem> SLGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<CascFileSystem>(environment.directory, environment.locale, environment.product, "Support Files\\listfile.csv"); //intentionally not appending 'Data'
	}

	std::unique_ptr<GameDatabase> SLGameClientAdaptor::database()
	{
		return std::make_unique<WDBDefsGameDatabase>(getClientInfo().environment.version);
	}

	const ModelSupport SLGameClientAdaptor::modelSupport()
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
				tmp->setCharacterEyeGlowHandler([](const Model* m) -> std::shared_ptr<CharEyeGlowGeosetModifier> { return std::make_shared<CharEyeGlowGeosetBasedGeosetModifier>(m); });
				return tmp;
			},
			[mf](GameFileSystem* fs, GameDatabase* db) {
				return std::make_unique<MergedAwareAttachmentCustomizationProvider>(fs, db, mf);
			}
		);
	}

	const GameClientInfo::Profile SLGameClientAdaptor::PROFILE{
		"Shadowlands",
		"SL",
		"9.x",
		{ 9, 0, 0, 0 }
	};

	std::unique_ptr<GameFileSystem> DFGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<CascFileSystem>(environment.directory, environment.locale, environment.product, "Support Files\\listfile.csv"); //intentionally not appending 'Data'
	}

	std::unique_ptr<GameDatabase> DFGameClientAdaptor::database()
	{
		return std::make_unique<WDBDefsGameDatabase>(getClientInfo().environment.version);
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
				tmp->setCharacterEyeGlowHandler([](const Model* m) -> std::shared_ptr<CharEyeGlowGeosetModifier> { return std::make_shared<CharEyeGlowGeosetBasedGeosetModifier>(m); });
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
		"10.x",
		{ 10, 0, 0, 0 }
	};

	std::unique_ptr<GameFileSystem> TWWGameClientAdaptor::filesystem(const GameClientInfo::Environment& environment)
	{
		return std::make_unique<CascFileSystem>(environment.directory, environment.locale, environment.product, "Support Files\\listfile.csv"); //intentionally not appending 'Data'
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
				tmp->setCharacterEyeGlowHandler([](const Model* m) -> std::shared_ptr<CharEyeGlowGeosetModifier> { return std::make_shared<CharEyeGlowGeosetBasedGeosetModifier>(m); });
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
		"11.x",
		{ 11, 0, 0, 0 }
	};

	std::unique_ptr<GameClientAdaptor> makeGameClientAdaptor(const GameClientInfo& info) {
		
		if (VanillaGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<VanillaGameClientAdaptor>(info);
		}

		if (TBCGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<TBCGameClientAdaptor>(info);
		}

		if (WOTLKGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<WOTLKGameClientAdaptor>(info);
		}

		if (CataGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<CataGameClientAdaptor>(info);
		}

		if (BFAGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<BFAGameClientAdaptor>(info);
		}

		if (SLGameClientAdaptor::PROFILE == info.profile) {
			return std::make_unique<SLGameClientAdaptor>(info);
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