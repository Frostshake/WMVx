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

	auto load_key = [&](auto key, auto defaultVal) {
		values.emplace(key, settings.value(key, defaultVal));
	};

	load_key(config::app::window_size, QSize(0, 0));
	load_key(config::app::background_color, QColorConstants::DarkGray);
	load_key(config::app::auto_focus_new_models, true);
	load_key(config::app::auto_animate_new_models, true);

	load_key(config::client::game_folder, "");

	load_key(config::exporter::last_image_directory, "");
	load_key(config::exporter::last_3d_directory, "");
	load_key(config::exporter::last_scene_directory, "");

	load_key(config::rendering::target_fps, int32_t(30));

	loaded = true;
}

void WMVxSettings::save()
{
	QSettings settings(fileName(), QSettings::IniFormat, this);

	for (const auto& item : values) {
		settings.setValue(item.first, item.second);
	}
}

QString WMVxSettings::fileName() const
{
	return QDir::currentPath() + QDir::separator() + "settings.ini";
}

bool WMVxSettings::isLoaded() const
{
	return loaded;
}
