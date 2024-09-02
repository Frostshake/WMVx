#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <WDBReader/Utility.hpp>
#include <optional>

class AssetSupportCache  : public QObject
{
	Q_OBJECT

public:

	static constexpr uint32_t UP_TO_DATE = 0;
	static constexpr uint32_t DBD_DEFS = 1 << 0;
	static constexpr uint32_t LIST_FILE = 1 << 1;
	static constexpr uint32_t TACT_KEYS = 1 << 2;
	static constexpr uint32_t ALL_OUTDATED = DBD_DEFS | LIST_FILE | TACT_KEYS;


	AssetSupportCache(QObject *parent);
	~AssetSupportCache();

	static const QString SupportDir;


	uint32_t status(const WDBReader::GameVersion& version) const;
	std::optional<QDateTime> lastUpdated() const;

	void updateDefinitions() const;
	void updateListFile() const;
	void updateTactKeys() const;

};
