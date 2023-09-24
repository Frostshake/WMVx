#pragma once

#include <optional>
#include "core/modeling/Scene.h"

class WidgetUsesScene
{
public:
	virtual ~WidgetUsesScene() {}

	virtual void onSceneLoaded(core::Scene* scene) {
		this->scene = scene;
	}

protected:
	WidgetUsesScene() {
		this->scene = nullptr;
	}

	core::Scene* scene;
};
