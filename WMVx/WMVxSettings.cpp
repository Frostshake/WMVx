#include "stdafx.h"
#include "WMVxSettings.h"

WMVxSettings* Settings::Singleton::_instance = nullptr;

WMVxSettings::WMVxSettings(QObject *parent)
	: QObject(parent),
	loaded(false)
{}

WMVxSettings::~WMVxSettings()
{}

void WMVxSettings::load()
{
	QSettings settings(fileName(), QSettings::IniFormat, this);
	windowSize = settings.value("app/window_size").toSize();
	backgroundColor = settings.value("app/background_color", QColorConstants::Black).value<QColor>();
	autoFocusNewModels = settings.value("app/auto_focus_new_models", true).toBool();
	autoAnimateNewModels = settings.value("app/auto_animate_newModels", true).toBool();
	gameFolder = settings.value("client/game_folder").toString();
	lastImageDirectory = settings.value("export/last_image_directory").toString();
	lastSceneDirectory = settings.value("export/last_scene_directory").toString();

	loaded = true;
}

void WMVxSettings::save()
{
	QSettings settings(fileName(), QSettings::IniFormat, this);
	settings.setValue("app/window_size", windowSize);
	settings.setValue("app/background_color", backgroundColor);
	settings.setValue("app/auto_focus_new_models", autoFocusNewModels);
	settings.setValue("app/auto_animate_new_models", autoAnimateNewModels);
	settings.setValue("client/game_folder", gameFolder);
	settings.setValue("export/last_image_directory", lastImageDirectory);
	settings.setValue("export/last_scene_directory", lastSceneDirectory);
}

QString WMVxSettings::fileName() const
{
	return QDir::currentPath() + QDir::separator() + "settings.ini";
}

bool WMVxSettings::isLoaded() const
{
	return loaded;
}
