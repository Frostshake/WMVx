#include "stdafx.h"
#include "AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	
	ui.labelBuildVersion->setText("Build version: " WMVX_VERSION);
}

AboutDialog::~AboutDialog()
{}
