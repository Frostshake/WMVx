#include "../../stdafx.h"
#include "Logger.h"

namespace core {

	Logger* Log::Singleton::_instance = nullptr;

	Logger::Logger(QObject* parent)
		: QObject(parent), file(nullptr)
	{
		auto path = getFilePath();

#ifndef _DEBUG
		file = new QFile(path);
		if (!file->open(QIODevice::WriteOnly | QIODevice::Text)) {
			delete file;
			file = nullptr;
		}
#endif
	}

	Logger::~Logger()
	{
		if (file != nullptr) {
			file->close();
			delete file;
		}
	}

	void Logger::message(QString message)
	{
		if (file != nullptr) {
			QTextStream stream(file);
			stream << message;
			Qt::endl(stream);
		}

		// logger might not be getting called from the UI thread, emit in wrapper to ensure its safe for UI.
		QMetaObject::invokeMethod(this, [this, message] {
			emit entryAdded(message);
		});
	}

	QString Logger::getFilePath()
	{
		return QDir::currentPath() + "/wmvx.log";
	}
};
