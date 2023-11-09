#include "../../stdafx.h"
#include <QDir>
#include <memory>
#include "GameClientInfo.h"
#include "GameClientAdaptor.h"
#include "../filesystem/MPQFileSystem.h"
#include "../filesystem/CascFileSystem.h"
#include "../database/VanillaGameDatabase.h"
#include "../modeling/VanillaModel.h"
#include "../database/WOTLKGameDatabase.h"
#include "../modeling/WOTLKModel.h"
#include "../database/BFAGameDatabase.h"
#include "../modeling/BFAModel.h"

namespace core {

	std::unique_ptr<GameFileSystem> VanillaGameClientAdaptor::filesystem(const QString& client_directory) {
		return std::make_unique<MPQFileSystem>(client_directory + QDir::separator() + "Data");
	}

	std::unique_ptr<GameDatabase> VanillaGameClientAdaptor::database()
	{
		return std::make_unique<VanillaGameDatabase>();
	}

	const ModelSupport VanillaGameClientAdaptor::modelSupport()
	{
		return ModelSupport(
			[]() {
				return std::make_unique<VanillaModel>();
			},
			[](GameFileSystem* fs) {
				return std::make_unique<LegacyTabardCustomisationProvider>(fs);
			}
		);
	}

	std::unique_ptr<GameFileSystem> WOTLKGameClientAdaptor::filesystem(const QString& client_directory)
	{
		return std::make_unique<MPQFileSystem>(client_directory + QDir::separator() + "Data");
	}

	std::unique_ptr<GameDatabase> WOTLKGameClientAdaptor::database()
	{
		return std::make_unique<WOTLKGameDatabase>(WOTLKGameDatabase());
	}

	const ModelSupport WOTLKGameClientAdaptor::modelSupport()
	{
		return ModelSupport(
			[]() {
				return std::make_unique<WOTLKModel>();
			},
			[](GameFileSystem* fs) {
				return std::make_unique<LegacyTabardCustomisationProvider>(fs);
			}
		);
	}

	std::unique_ptr<GameFileSystem> BFAGameClientAdaptor::filesystem(const QString& client_directory)
	{
		return std::make_unique<CascFileSystem>(client_directory, "Support Files\\bfa\\listfile.csv"); //intentionally not appending 'Data'
	}

	std::unique_ptr<GameDatabase> BFAGameClientAdaptor::database()
	{
		return std::make_unique<BFAGameDatabase>();
	}

	const ModelSupport BFAGameClientAdaptor::modelSupport()
	{
		return ModelSupport(
			[]() {
				return std::make_unique<BFAModel>(BFAModel());
			},
			[](GameFileSystem* fs) {
				return std::make_unique<ModernTabardCustomizationProvider>(fs);
			}
		);
	}
}