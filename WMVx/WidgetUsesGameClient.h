#pragma once
#include "core/game/GameClientAdaptor.h"

class WidgetUsesGameClient
{
public:
	virtual ~WidgetUsesGameClient() {};

	virtual void onGameConfigLoaded(core::GameDatabase* db, core::GameFileSystem* fs, core::ModelSupport& ms) {
		gameDB = db;
		gameFS = fs;
		modelSupport = ms;
	}

protected:
	WidgetUsesGameClient() : 
		gameDB(nullptr), gameFS(nullptr), modelSupport(core::NullModelSupport) {
	}

	core::GameDatabase* gameDB;
	core::GameFileSystem* gameFS;
	core::ModelSupport modelSupport;

};
