#include <QtGui/QApplication>
#include "mainwindow.h"
#include <ipcameracapture.h>
#include <eventtracker.h>
#include <dispatcher.h>
#include <controller.h>
#include <VideoLoader.h>
#include <DBManager.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	app.setStyle("plastique");
	Dispatcher dispatcher;

	MainWindow w(NULL, IPCameraCapture::getCameraList(), &dispatcher);
	Controller controller(&dispatcher, &w);
	IPCameraCapture ipCameraCapture(&dispatcher);
	EventTracker tracker(&dispatcher);
	VideoLoader videoLoader(&dispatcher);
	DBManager dbmanager(&dispatcher);

	w.show();
	int returnvalue = app.exec();

	pthread_join(controller.get_thread(), NULL);
	pthread_join(ipCameraCapture.get_thread(), NULL);
	pthread_join(tracker.get_thread(), NULL);
	pthread_join(videoLoader.get_thread(), NULL);
	pthread_join(dbmanager.get_thread(), NULL);

	return returnvalue;
}
