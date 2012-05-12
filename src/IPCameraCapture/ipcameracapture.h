#ifndef IPCAMERACAPTURE_H
#define IPCAMERACAPTURE_H

#include <QString>
#include <QImage>
#include <QHttp>
#include <cv.h>
#include <vector>
#include <QDateTime>
#include <QSharedPointer>
#include <tracker_types.h>
#include <signalprocessor.h>
#include <dispatcher.h>
#include <tracker_types.h>
#include <map>
extern "C"
{
#include <FiniteStateMachine.h>
}
class IPCameraCapture : public QObject,
                        public SignalProcessor
{
	Q_OBJECT
	private:
		QHttp http;
		QByteArray localBuffer;
		double fps;
		unsigned int frameCounter;
		QDateTime lastFPSUpdate;
		QByteArray SOI, EOI;
		int indexOfSOI, lastEOICheck;
		static std::map<QString, QString> cameraToPath;
		Transition READY_TO_CONNECT_REQUESTED,
			   CONNECT_REQUESTED_TO_STREAMING,
			   CONNECT_REQUESTED_TO_READY,
			   STREAMING_TO_STOP_REQUESTED,
			   CONNECT_REQUESTED_TO_STOP_REQUESTED, 
			   STREAMING_TO_READY,
			   STOP_REQUESTED_TO_READY;

	public:
		IPCameraCapture(Dispatcher* _dispatcher);
		static std::vector<QString> getCameraList();
		~IPCameraCapture();

	protected:
		void process(int code, void *param);
		void init_fsm(FiniteStateMachine fsm);
		void destroy_pending_task(int code, void *param);
		void handle_transition(Transition transition, void* param);

	signals:
		void abort();

		public slots:
		void readyRead ( const QHttpResponseHeader & resp );
		void requestFinished(int, bool);
		void ConnectToCamera(Camera_t *);
		void StopCapturing();
		void httpStateChanged(int state);

	private:
		static void generateCameraToPathMap();

};

#endif // IPCAMERACAPTURE_H
