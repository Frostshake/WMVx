#pragma once

#include <QObject>
#include "Singleton.h"

namespace core {

	class Logger : public QObject
	{
		Q_OBJECT

	public:
		Logger(QObject* parent);
		~Logger();

		void message(QString message);

		QString getFilePath();

	signals:
		void entryAdded(QString message);

	private:
		QFile* file;
	};

	class Log : public Singleton<Logger> {
	public:
		static void message(QString message) {
			assert(_instance != nullptr);
			if (_instance != nullptr) {
				_instance->message(message);
			}
		}
	};

}