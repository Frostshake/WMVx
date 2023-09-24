#pragma once

#include "core/utility/Singleton.h"
#include <QObject>

class WMVxSettings : public QObject
{
	Q_OBJECT

public:
	WMVxSettings(QObject *parent);
	~WMVxSettings();

	void load();
	void save();

	QString fileName() const;
	bool isLoaded() const;

	//app
	QSize windowSize;
	QColor backgroundColor;

	bool autoFocusNewModels;
	bool autoAnimateNewModels;

	//client
	QString gameFolder;

	//export
	QString lastImageDirectory;
	QString lastSceneDirectory;

protected: 
	bool loaded;
};

class Settings : public core::Singleton<WMVxSettings> {
public:
	static QString gameFolder() {
		assert(_instance != nullptr);
		return _instance->gameFolder;
	}

	static QSize windowSize() {
		assert(_instance != nullptr);
		return _instance->windowSize;
	}

	static QColor backgroundColor() {
		assert(_instance != nullptr);
		return _instance->backgroundColor;
	}

	static bool autoFocusNewModels() {
		assert(_instance != nullptr);
		return _instance->autoFocusNewModels;
	}

	static bool autoAnimateNewModels() {
		assert(_instance != nullptr);
		return _instance->autoAnimateNewModels;
	}

	static QString lastImageDirectory() {
		assert(_instance != nullptr);
		return _instance->lastImageDirectory;
	}

	static QString lastSceneDirectory() {
		assert(_instance != nullptr);
		return _instance->lastSceneDirectory;
	}
};