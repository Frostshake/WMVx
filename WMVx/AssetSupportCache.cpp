#include "stdafx.h"
#include "AssetSupportCache.h"

AssetSupportCache::AssetSupportCache(QObject *parent)
	: QObject(parent)
{}

AssetSupportCache::~AssetSupportCache()
{}

uint32_t AssetSupportCache::status(const WDBReader::GameVersion & version) const
{
	//if (Settings::get<bool>(config::app::support_auto_update)) {
	//TODO if auto update is set, always refresh so often.

	//TODO


	return AssetSupportCache::UP_TO_DATE;
}

std::optional<QDateTime> AssetSupportCache::lastUpdated() const
{
	//TODO
	return QDateTime::currentDateTime();
}

void AssetSupportCache::updateDefinitions() const
{
	//TODO
}

void AssetSupportCache::updateListFile() const
{
	//TODO
}

void AssetSupportCache::updateTactKeys() const
{
	//TODO
}

const QString AssetSupportCache::SupportDir = "Support Files";