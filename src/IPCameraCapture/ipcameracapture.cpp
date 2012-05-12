#include <ipcameracapture.h>
#include <QImageReader>
#include <cv.h>
#include <highgui.h>
#include <QHttpRequestHeader>
#include <SignalCode.h>
#include <stdio.h>

static const char soi[] = {(char)0xff, (char)0xd8}; /*Start Of Image Flag*/
static const char eoi[] = {(char)0xff, (char)0xd9}; /*End Of Image Flag*/

IPCameraCapture::IPCameraCapture(Dispatcher* _dispatcher):SignalProcessor("ipcameracapture", _dispatcher)
{
	indexOfSOI = -1;
	lastEOICheck = -1;

	QObject::connect(&http, SIGNAL(readyRead(QHttpResponseHeader)), this, SLOT(readyRead(QHttpResponseHeader)));
	QObject::connect(&http, SIGNAL(requestFinished(int,bool)), this, SLOT(requestFinished(int,bool)));
	QObject::connect(this, SIGNAL(abort()), &http, SLOT(abort()));
	QObject::connect(&http, SIGNAL(stateChanged(int)), this, SLOT(httpStateChanged(int)));

	SOI = QByteArray::fromRawData(soi, 2);
	EOI = QByteArray::fromRawData(eoi, 2);
	_init_fsm(this);
}

void
IPCameraCapture::init_fsm(FiniteStateMachine fsm)
{
	fsm_add_state(fsm, "connect-requested");
	fsm_add_state(fsm, "streaming");
	fsm_add_state(fsm, "stop-requested");
	READY_TO_CONNECT_REQUESTED = fsm_link_states(fsm, "connect", "ready", "connect-requested");
	CONNECT_REQUESTED_TO_STREAMING = fsm_link_states(fsm, "success", "connect-requested", "streaming");
	CONNECT_REQUESTED_TO_READY = fsm_link_states(fsm, "aborted", "connect-requested", "ready");
	CONNECT_REQUESTED_TO_STOP_REQUESTED = fsm_link_states(fsm, "stop", "connect-requested", "stop-requested");
	STREAMING_TO_STOP_REQUESTED = fsm_link_states(fsm, "stop", "streaming", "stop-requested");
	STREAMING_TO_READY = fsm_link_states(fsm, "aborted", "streaming", "ready");
	STOP_REQUESTED_TO_READY = fsm_link_states(fsm, "aborted", "stop-requested", "ready");

}

void 
IPCameraCapture::ConnectToCamera(Camera_t *camInf)
{
	generateCameraToPathMap();
	lastFPSUpdate = QDateTime::currentDateTime();

	http.setHost(camInf->cam_uri,QHttp::ConnectionModeHttp, 80);

	/*  Note:
	 *  we need to create the HTTP Request header,
	 *  There seems to be a bug in Qt which causes
	 *  problems with the authentication to some of
	 *  the cameras
	 */
	QHttpRequestHeader header(QLatin1String("GET"), cameraToPath[camInf->cam_brand]);
	header.setValue(QLatin1String("Connection"), QLatin1String("Keep-Alive"));
	QString user = camInf->cam_username;
	QString pass = camInf->cam_password;
	if (!user.isEmpty())
	{
		QByteArray authpass = user.toAscii();
		if (!pass.isEmpty()) {
			authpass += ':';
			authpass += pass.toAscii();
		}
		header.setValue(QLatin1String("Authorization"), QLatin1String("Basic " + authpass.toBase64()));
	}
	header.setValue(QLatin1String("Host"), camInf->cam_uri);
	http.request(header);

}


void
IPCameraCapture::requestFinished(int , bool err)
{
	if (err)
	{
		printf ("there was an error processing something in ipcameracapture :: requestFinished\n");
		emit abort();
		dispatcher->send_signal("ipcameracapture", ABORTED, NULL);
	}
}


void 
IPCameraCapture::StopCapturing()
{
	emit abort();
}


void 
IPCameraCapture::readyRead ( const QHttpResponseHeader & resp )
{
	dispatcher->send_signal("ipcameracapture", CONNECTED, NULL);

	localBuffer.append(http.readAll());
	//check weather SOI is available
	if (indexOfSOI == -1)
	{
		indexOfSOI = localBuffer.indexOf(SOI, 0);
		lastEOICheck = indexOfSOI;
		if (indexOfSOI == -1)
			return;
	}
	//check weather EOI is available
	int indexOfEOI = localBuffer.indexOf(EOI, lastEOICheck);
	if (indexOfEOI == -1)
	{
		lastEOICheck = localBuffer.size()-3;
		return;
	}

	QByteArray imageBuffer = localBuffer.left(indexOfEOI+2).right(indexOfEOI-indexOfSOI+2);
	indexOfSOI = -1;
	lastEOICheck = -1;
	localBuffer.remove(0, indexOfEOI);
	QDataStream stream(&imageBuffer, QIODevice::ReadOnly);

	//create a decoder object
	QImageReader reader(stream.device(), "jpeg");

	//retreieve the image
	QImage* imgPtr = new QImage();

	reader.read(imgPtr);

	if (imgPtr->isNull())
		delete imgPtr;
	else
		dispatcher->send_signal("controller", NEW_IMAGE, imgPtr);

	//    /*figure out the FPS*/
	//    frameCounter++;
	//    QDateTime currentTime = QDateTime::currentDateTime();
	//    qint64 timeDiff = currentTime.toMSecsSinceEpoch() - lastFPSUpdate.toMSecsSinceEpoch();
	//    fps = (double) (frameCounter) / ((double) (timeDiff) / 1000.0);
	//    emit updateFps(fps);
	//    /*reset the frameCounter every 5 seconds*/
	//    if (timeDiff >= 5000)
	//    {
	//          frameCounter = 0;
	//          lastFPSUpdate = QDateTime::currentDateTime();
	//    }

}

void
IPCameraCapture::generateCameraToPathMap()
{
	if (cameraToPath.size() == 0)
	{
		cameraToPath["Panasonic"] = "/nphMotionJpeg?Resolution=320x240&Quality=Standard";
		cameraToPath["Airlink101"] = "/cgi/mjpg/mjpeg.cgi";
		cameraToPath["Axis"] = "/axis-cgi/mjpg/video.cgi?resolution=640x480&camera=1";
		cameraToPath["DLink"] = "/mjpeg.cgi";
		cameraToPath["Cannon"] = "/-wvhttp-01-/getoneshot?frame_count=no_limit";
		cameraToPath["Gadspot"] = "/GetData.cgi?Status=0";
		cameraToPath["Mobotix"] = "/control/faststream.jpg?stream=full";
		cameraToPath["Linksys"] = "/img/video.mjpeg";
		cameraToPath["Qnap"] = "/cgi/mjpg/mjpeg.cgi";
		cameraToPath["Sony"] = "/image";
		cameraToPath["Toshiba"] = "/getstream.cgi?10&&&&0&0&0&0&0";
		cameraToPath["webcamXP"] = "/cam_1.cgi";
		cameraToPath["Ycam"] = "/stream.jpg";
	}
}

std::map<QString, QString> IPCameraCapture::cameraToPath;
std::vector<QString> IPCameraCapture::getCameraList()
{
	generateCameraToPathMap();
	std::vector<QString> cameras;
	for (std::map<QString, QString>::iterator iter = cameraToPath.begin(); iter != cameraToPath.end(); iter++)
		cameras.push_back(iter->first);
	return cameras;
}

void 
IPCameraCapture::httpStateChanged(int state)
{
	if (state == QHttp::Unconnected)
	{
		localBuffer.clear();
		printf ("IP Camera Capture: connection has failed...\n");
		dispatcher->send_signal("ipcameracapture", ABORTED, NULL);
	}
}

void 
IPCameraCapture::handle_transition(Transition transition, void* param)
{
	if (transition == READY_TO_CONNECT_REQUESTED)
	{
		Camera_t* camInfo = reinterpret_cast<Camera_t*>(param);
		ConnectToCamera(camInfo);
		destroy_pending_task(START, param);
	}
	else if (transition == CONNECT_REQUESTED_TO_READY || transition == STREAMING_TO_READY)
		dispatcher->send_signal("controller", FAILURE, NULL);
	else if (transition == STREAMING_TO_STOP_REQUESTED || transition == CONNECT_REQUESTED_TO_STOP_REQUESTED)
		emit abort();
}

void 
IPCameraCapture::destroy_pending_task(int code, void *param)
{
	switch (code)
	{
		case START:
			{
				Camera_t* camInfo = reinterpret_cast<Camera_t*>(param);
				free(camInfo->cam_brand);
				free(camInfo->cam_password);
				free(camInfo->cam_uri);
				free(camInfo->cam_username);
				free(camInfo->vid_dir);
				delete camInfo;
				break;
			}
	}
}

void 
IPCameraCapture::process(int code, void *param)
{
	switch (code)
	{
		case START:
			{
				if (!apply_event("connect", param))
					destroy_pending_task(code, param);
				break;
			}
		case ABORTED:
			{
				apply_event("aborted", NULL);
				break;
			}
		case CONNECTED:
			{
				apply_event("success", NULL);
				break;
			}
		case STOP:
			{
				apply_event("stop", NULL);
				break;
			}
	}
}

IPCameraCapture::~IPCameraCapture()
{
}
