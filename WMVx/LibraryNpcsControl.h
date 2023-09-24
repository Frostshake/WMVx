#pragma once

#include <QWidget>
#include "ui_LibraryNpcsControl.h"
#include "core/utility/Logger.h"
#include "WidgetUsesScene.h"
#include "WidgetUsesGameClient.h"
#include "Debounce.h"

class LibraryNpcsControl : public QWidget, public WidgetUsesScene, public WidgetUsesGameClient
{
	Q_OBJECT

public:
	LibraryNpcsControl(QWidget* parent = nullptr);
	~LibraryNpcsControl();

	void onGameConfigLoaded(core::GameDatabase* db, core::GameFileSystem* fs, core::ModelSupport& ms) override;

protected:
	virtual bool event(QEvent* event);

private:
	Ui::LibraryNpcsControlClass ui;

	Debounce* delayedSearch;

	void loadNpcs();
};
