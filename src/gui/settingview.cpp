#include <settingview.h>

SettingView::SettingView(QWidget* parent):QWidget(parent)
{
	init();
}

void
SettingView::init()
{
	tabWidget = new QTabWidget(this);
	sourceWidget = new SourceView(this);
	databaseWidget = new QWidget();

	tabWidget->addTab(sourceWidget, "Source");
	tabWidget->addTab(databaseWidget, "Database");
}
