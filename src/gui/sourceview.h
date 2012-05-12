#ifndef _SOURCE_VIEW_
#define _SOURCE_VIEW_

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

class SourceView : public QWidget
{
	Q_OBJECT
	public:
		SourceView(QWidget* parent);
		void 				init();
	private:
		QLineEdit* 	uriEdit;
		QLineEdit* 	usernameEdit;
		QLineEdit* 	passwordEdit;
		QComboBox* 	brandComboBox;
		QComboBox* 	resolutionComboBox;
		QLineEdit* 	outputDirEdit;
		QPushButton* 	browseBtn;
};

#endif
