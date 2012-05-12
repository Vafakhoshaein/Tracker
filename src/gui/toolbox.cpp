#include <toolbox.h>

ToolBox::ToolBox(QWidget* parent):QToolBox(parent)
{
	init();
}

void
ToolBox::init()
{
	settingWidget = new SettingView(this);
	viewWidget = new QWidget(this);
	logWidget = new QWidget(this);
	addItem(settingWidget, "Settings");
	addItem(viewWidget, "View");
	addItem(logWidget, "Log");
}
