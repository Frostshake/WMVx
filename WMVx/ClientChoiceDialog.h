#pragma once

#include <QDialog>
#include "ui_ClientChoiceDialog.h"
#include "core/game/GameClientInfo.h"

class ClientChoiceDialog : public QDialog
{
	Q_OBJECT

public:
	ClientChoiceDialog(QWidget *parent = nullptr);
	~ClientChoiceDialog();

signals:
	void chosen(core::GameClientInfo info);

private:
	Ui::ClientChoiceDialogClass ui;

	void load();
	void detectVersion();
	
	static const std::array<const core::GameClientInfo::Profile*, 8> availableProfiles;
};
