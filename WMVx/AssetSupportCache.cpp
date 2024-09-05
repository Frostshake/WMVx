#include "stdafx.h"
#include "AssetSupportCache.h"
#include "core/utility/Logger.h"
#include <queue>
#include <QFile>
#include "WMVxSettings.h"
#include <fstream>
#include <WDBReader/WoWDBDefs.hpp>

AssetSupportCache::AssetSupportCache(QObject *parent)
	: QObject(parent)
{
}

AssetSupportCache::~AssetSupportCache()
{}

uint32_t AssetSupportCache::status(const WDBReader::GameVersion & version) const
{
	if (Settings::get<bool>(config::app::support_auto_update)) {
		const auto last_updated = lastUpdated();
		if (!last_updated) {
			return AssetSupportCache::ALL_OUTDATED;
		}
		const auto current = QDateTime::currentDateTime();
		const auto diff = last_updated.value().secsTo(current);
		if (diff > (60 * 60 * 24 * 7)) {
			return AssetSupportCache::ALL_OUTDATED;
		}
	}

	std::ifstream stream(SupportDir.toStdString() + "/definitions/ChrRaces.dbd");
	auto definition = WDBReader::WoWDBDefs::DBDReader::read(stream);
	auto schema = WDBReader::WoWDBDefs::makeSchema(definition, version);

	if (!schema.has_value()) {
		AssetSupportCache::ALL_OUTDATED;
	}

	return AssetSupportCache::UP_TO_DATE;
}

std::optional<QDateTime> AssetSupportCache::lastUpdated() const
{

	std::optional<QDateTime> result;

	auto try_file = [&result](const auto& name) {
		const QFileInfo info(name);
		const QDateTime lastModified = info.lastModified();

		if (!result.has_value() || lastModified < *result) {
			result = lastModified;
		}
	};

	try_file(SupportDir + "/listfile.csv");
	try_file(SupportDir + "/tactkeys.txt");
	try_file(SupportDir + "/definitions/ChrRaces.dbd");

	return result;
}


bool AssetSupportCache::fetchUpdates(uint32_t status, std::stop_token stop)
{
	bool all_success = true;
	

	QNetworkAccessManager* manager = new QNetworkAccessManager(this);

	std::queue<AssetSupportCache::Update> updates;

	if (status & AssetSupportCache::LIST_FILE) {
		updates.emplace(
			"https://github.com/wowdev/wow-listfile/releases/latest/download/community-listfile.csv",
			SupportDir + "/listfile.csv",
			"List file"
		);
	}

	if (status & AssetSupportCache::TACT_KEYS) {
		updates.emplace(
			"https://raw.githubusercontent.com/wowdev/TACTKeys/master/WoW.txt", 
			SupportDir + "/tactkeys.txt",
			"TACT keys"
		);
	}

	if (status & AssetSupportCache::DBD_DEFS) {

		auto make = [&](const auto & dbd_name) {
			updates.emplace(
				QString("https://raw.githubusercontent.com/wowdev/WoWDBDefs/master/definitions/%1").arg(dbd_name),
				SupportDir + "/definitions/" + dbd_name,
				dbd_name
			);
		};

		make("AnimationData.dbd");
		make("CharComponentTextureLayouts.dbd");
		make("CharComponentTextureSections.dbd");
		make("ChrCustomization.dbd");
		make("ChrCustomizationChoice.dbd");
		make("ChrCustomizationElement.dbd");
		make("ChrCustomizationGeoset.dbd");
		make("ChrCustomizationMaterial.dbd");
		make("ChrCustomizationOption.dbd");
		make("ChrCustomizationSkinnedModel.dbd");
		make("ChrModel.dbd");
		make("ChrModelTextureLayer.dbd");
		make("ChrRaces.dbd");
		make("ChrRaceXChrModel.dbd");
		make("ComponentModelFileData.dbd");
		make("ComponentTextureFileData.dbd");
		make("Creature.dbd");
		make("CreatureDisplayInfo.dbd");
		make("CreatureDisplayInfoExtra.dbd");
		make("CreatureModelData.dbd");
		make("GuildTabardBackground.dbd");
		make("GuildTabardBorder.dbd");
		make("GuildTabardEmblem.dbd");
		make("Item.dbd");
		make("ItemAppearance.dbd");
		make("ItemDisplayInfo.dbd");
		make("ItemDisplayInfoMaterialRes.dbd");
		make("ItemModifiedAppearance.dbd");
		make("ItemSparse.dbd");
		make("ModelFileData.dbd");
		make("TextureFileData.dbd");
	}
	
	while (!updates.empty()) {
		if (stop.stop_requested()) {
			all_success = false;
			break;
		}

		auto update = updates.front();
		updates.pop();

		QEventLoop loop;

		QNetworkRequest request;
		request.setUrl(update.url);
		QNetworkReply* reply = manager->get(request);

		connect(reply, &QNetworkReply::downloadProgress, [this, update, &stop, reply](qint64 received, qint64 total) {
			if (stop.stop_requested()) {
				reply->abort();
			}

			emit progress(&update, received, total);
		});

		connect(reply, &QNetworkReply::errorOccurred, &loop, [&all_success, this, update](QNetworkReply::NetworkError code) {
			all_success = false;
			emit error(&update, "Network error.");
		});

		connect(reply, &QNetworkReply::finished, [reply, this, update, &loop]() {
			if (reply->error() == QNetworkReply::NoError) {
				QFile file(update.dest);
				if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
					file.write(reply->readAll());
					file.close();
				}
				else {
					core::Log::message("Update failure - cannot write file - " + update.dest);
				}
			}
			else {
				core::Log::message("Update failure - Network error finished - " + update.url.toString());
				emit error(&update, "Network error finished.");
			}
			reply->deleteLater();
			loop.quit();
		});


		loop.exec();
		QThread::msleep(250);
	}

	return all_success;
}

const QString AssetSupportCache::SupportDir = "Support Files";