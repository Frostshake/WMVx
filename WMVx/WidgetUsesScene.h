#pragma once

#include <optional>
#include "core/modeling/Scene.h"

class WidgetUsesScene
{
public:
	virtual ~WidgetUsesScene() {}

	virtual void onSceneLoaded(core::Scene* new_scene) {
		this->scene = new_scene;
	}

protected:
	WidgetUsesScene() {
		this->scene = nullptr;
	}

	core::Scene* scene;
};
