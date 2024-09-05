#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <WDBReader/Utility.hpp>
#include <optional>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <stop_token>

class AssetSupportCache  : public QObject
{
	Q_OBJECT

public:

	struct Update {
	public:
		Update(QString u, QString d, QString n) : url(u), dest(d), name(n) {}

		QUrl url;
		QString dest;
		QString name;
	};

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

	bool fetchUpdates(uint32_t status, std::stop_token stop);


signals:
	void progress(const Update* update, uint64_t received, uint64_t total);
	void error(const Update* update, QString message);


};
