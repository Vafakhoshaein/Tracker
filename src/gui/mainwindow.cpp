#include "mainwindow.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <SignalCode.h>
#include <tracker_types.h>
#include <string.h>
#include <QPixmap>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>


MainWindow::MainWindow(QWidget *parent, vector<QString> cameraList, Dispatcher* _dispatcher): QMainWindow(parent)
{
	init(cameraList);
	dispatcher = _dispatcher;

	centralizeFrame();
	connectSignals();
	update();
}

void
MainWindow::init(vector<QString> cameraList)
{
	startBtn = new QPushButton("Start", this);
	stopBtn = new QPushButton("Stop", this);
	exitBtn = new QPushButton("Exit", this);
	toolbox = new ToolBox(this);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	QHBoxLayout* lowerLayout = new QHBoxLayout();

	lowerLayout->addWidget(startBtn);
	lowerLayout->addWidget(stopBtn);
	lowerLayout->addStretch();
	lowerLayout->addWidget(exitBtn);
	mainLayout->addWidget(toolbox);
	mainLayout->addLayout(lowerLayout);
	QWidget* cw = new QWidget();
	cw->setLayout(mainLayout);
	setCentralWidget(cw);
}

void
MainWindow::connectSignals()
{
	connect(startBtn, SIGNAL(clicked()), this, SLOT(on_startBtn_clicked()));
	connect(stopBtn, SIGNAL(clicked()), this, SLOT(on_stopBtn_clicked()));
	connect(exitBtn, SIGNAL(clicked()), this, SLOT(on_exitBtn_clicked()));

	connect(this, SIGNAL(_updateByState(char*)), this, SLOT(updateState(char*)));
	connect(this, SIGNAL(_updateImagePair(QImage*,QImage*)), this, SLOT(updateImagePair(QImage*,QImage*)));
}


void
MainWindow::centralizeFrame()
{
	QRect frect = frameGeometry();
	frect.moveCenter(QDesktopWidget().availableGeometry().center());
	move(frect.topLeft());
}

void 
MainWindow::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::warning(this, "Quit?",
				"Would you like to exit the tracker system application?",
				QMessageBox::Ok,
				QMessageBox::Cancel)== QMessageBox::Ok)
	{
		event->accept();
		dispatcher->send_signal("controller", EXIT_BTN_CLICKED, NULL);
	}
	else
	{
		event->ignore();
	}
}

void 
MainWindow::updateUI_ImagePair(QImage *img1, QImage *img2)
{
	emit _updateImagePair(img1, img2);
}

void 
MainWindow::updateImagePair(QImage* img1, QImage* img2)
{
	delete img1;
	delete img2;
}

void 
MainWindow::updateState(char* state)
{
	bool ready = !strcmp(state, "ready");
	free(state);
}

void 
MainWindow::stateReport(char* state)
{
	emit _updateByState(state);
}

MainWindow::~MainWindow()
{
}

void 
MainWindow::on_exitBtn_clicked()
{
	close();
}

void 
MainWindow::on_startBtn_clicked()
{

}

void 
MainWindow::on_stopBtn_clicked()
{
	dispatcher->send_signal("controller", STOP_BTN_CLICKED, NULL);
}

void 
MainWindow::DisplayError(QString err)
{
	QMessageBox::warning(this, "Error", err, QMessageBox::Ok, QMessageBox::NoButton);
}


