#ifndef 	__TOOLBOX_H_
#define 	__TOOLBOX_H_

#include <QToolBox>
#include <QWidget>
#include <settingview.h>

class ToolBox : public QToolBox
{
	Q_OBJECT
	public:
		ToolBox(QWidget* parent);
		void			init();

	private:
		SettingView* 		settingWidget;
		QWidget* 		viewWidget;
		QWidget* 		logWidget;
};

#endif
