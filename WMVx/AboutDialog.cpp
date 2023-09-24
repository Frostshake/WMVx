#include "stdafx.h"
#include "AboutDialog.h"
#include "Build.h"

AboutDialog::AboutDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	
	ui.labelBuildVersion->setText("Build version: " WMVX_BUILD);
}

AboutDialog::~AboutDialog()
{}
