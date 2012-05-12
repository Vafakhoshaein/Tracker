#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QCloseEvent>
#include <dispatcher.h>
#include <QLineEdit>
#include <toolbox.h>
#include <QPushButton>

extern "C"
{
#include <FiniteStateMachine.h>
}

using namespace std;

class MainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit 		MainWindow(QWidget *parent = 0, vector<QString> cameraList = vector<QString>(),Dispatcher* d = 0);
		void 			closeEvent(QCloseEvent* event);
		~MainWindow();
		void 			stateReport(char* state);
		void			updateUI_ImagePair(QImage* img1, QImage* img2);
		bool 			isRecordVideoChkBoxChecked() {return false;}
		bool 			isLogTargetsChkBoxChecked() {return false;}
		QString 		getLogTargetFilename() {return QString();}


	private:
		void 			DisplayError(QString);
		void 			connectSignals();
		void 			centralizeFrame();
		Dispatcher* 		dispatcher;
		ToolBox* 		toolbox;
		QPushButton* 		startBtn;
		QPushButton* 		stopBtn;
		QPushButton* 		exitBtn;


	signals:
		void 			_updateByState(char*);
		void 			_updateImagePair(QImage* firstImage, QImage* secondImage);


	public slots:
		void 			init(vector<QString> cameraList);
		void 			updateImagePair(QImage* firstImage, QImage* secondImage);
		void 			updateState(char*);
		void 			on_startBtn_clicked();
		void 			on_stopBtn_clicked();
		void 			on_exitBtn_clicked();
};

#endif // MAINWINDOW_H
