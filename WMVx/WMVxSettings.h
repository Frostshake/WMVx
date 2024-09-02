#pragma once

#include "core/utility/Singleton.h"
#include <QObject>

#define WMVX_CONFIG_KEY(group, key) namespace config::group { constexpr const char* key = #group"/"#key; };

WMVX_CONFIG_KEY(app, window_size)
WMVX_CONFIG_KEY(app, window_maximized)
WMVX_CONFIG_KEY(app, background_color)
WMVX_CONFIG_KEY(app, auto_focus_new_models)
WMVX_CONFIG_KEY(app, auto_animate_new_models)
WMVX_CONFIG_KEY(app, support_auto_update)

WMVX_CONFIG_KEY(client, game_folder)

WMVX_CONFIG_KEY(exporter, last_image_directory)
WMVX_CONFIG_KEY(exporter, last_3d_directory)
WMVX_CONFIG_KEY(exporter, last_scene_directory)

WMVX_CONFIG_KEY(rendering, target_fps)
WMVX_CONFIG_KEY(rendering, camera_type);
WMVX_CONFIG_KEY(rendering, camera_hide_mouse);

#undef WMVX_CONFIG_KEY


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

	template<typename T = QString>
	T get(const char* key) const {
		return values.at(key).value<T>();
	}

	template<typename T>
	void set(const char* key, T val) {
		values[key] = val;
	}

protected: 
	std::map<QString, QVariant> values;
	bool loaded;
};

class Settings : public core::Singleton<WMVxSettings> {
public:
	template<typename T = QString>
	static T get(const char* key) {
		assert(_instance != nullptr);
		return _instance->get<T>(key);
	}
};