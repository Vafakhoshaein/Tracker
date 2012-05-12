#ifndef _SETTING_VIEW_H_
#define _SETTING_VIEW_H_

#include <QTabWidget>
#include <sourceview.h>

class SettingView : public QWidget
{
	Q_OBJECT
	public:
		SettingView(QWidget* parent);
		void 				init();

	private:
		QTabWidget* 			tabWidget;
		SourceView* 			sourceWidget;
		QWidget* 			databaseWidget;
};

#endif
