#include <VideoLoader.h>
#include <SignalCode.h>
#include "cv.h"
#include "highgui.h"
#include <common.h>

VideoLoader::VideoLoader(Dispatcher* _dispatcher):SignalProcessor("videoloader", _dispatcher),
						  capture(0)
{
	_init_fsm(this);
	pthread_mutex_init(&capture_mutex, NULL);
	pthread_cond_init(&capture_cond, NULL);
	pthread_create(&streamingThread, NULL, VideoLoader::streamingThreadFunc, this);
}

void
VideoLoader::init_fsm(FiniteStateMachine fsm)
{
	fsm_add_state(fsm, "connect-requested");
	fsm_add_state(fsm, "streaming");
	fsm_add_state(fsm, "stop-requested");
	READY_TO_CONNECT_REQUESTED = fsm_link_states(fsm, "connect", "ready", "connect-requested");
	CONNECT_REQUESTED_TO_STREAMING = fsm_link_states(fsm, "success", "connect-requested", "streaming");
	CONNECT_REQUESTED_TO_READY = fsm_link_states(fsm, "aborted", "connect-requested", "ready");
	STREAMING_TO_STOP_REQUESTED = fsm_link_states(fsm, "stop", "streaming", "stop-requested");
	STREAMING_TO_READY = fsm_link_states(fsm, "aborted", "streaming", "ready");
	STOP_REQUESTED_TO_READY = fsm_link_states(fsm, "aborted", "stop-requested", "ready");


}
void
VideoLoader::process(int code, void* param)
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

	void
VideoLoader::destroy_pending_task(int code, void *param)
{
	switch (code)
	{
		case START:
			{
				Camera_t* camInfo = reinterpret_cast<Camera_t*>(param);
				free(camInfo->vid_filename);
				free(camInfo->vid_dir);
				delete camInfo;
				break;
			}
	}
}


void*
VideoLoader::streamingThreadFunc(void* param)
{
	VideoLoader* videoLoader = reinterpret_cast<VideoLoader*> (param);
	while (1)
	{
		pthread_mutex_lock(&videoLoader->capture_mutex);
		if (!videoLoader->capture)
			pthread_cond_wait(&videoLoader->capture_cond, &videoLoader->capture_mutex);
		if (videoLoader->capture)
		{
			double fps = cvGetCaptureProperty(videoLoader->capture, CV_CAP_PROP_FPS);
			struct timespec tf, t2, r;
			tf.tv_sec = 0;
			tf.tv_nsec = 1000000000L / fps;
			GET_ABSOLUTE_TIME(t2);
			while (videoLoader->capture)
			{
				IplImage* img = cvQueryFrame(videoLoader->capture);
				if (!img)
				{
					cvReleaseCapture(&videoLoader->capture);
					videoLoader->dispatcher->send_signal("videoloader", ABORTED, NULL);
				}
				else
				{
					videoLoader->dispatcher->send_signal("controller", NEW_IMAGE, IplImage2QImage(img));
					/*send the frame*/
					ADD_TIME(r, t2, tf);
					pthread_cond_timedwait(&videoLoader->capture_cond, &videoLoader->capture_mutex, &r);
					GET_ABSOLUTE_TIME(t2);
				}
			}
		}
		else
		{
			pthread_mutex_unlock(&videoLoader->capture_mutex);
			break;
		}
		pthread_mutex_unlock(&videoLoader->capture_mutex);
	}
}

void 
VideoLoader::handle_transition(Transition transition, void* param)
{


	if (transition == READY_TO_CONNECT_REQUESTED)
	{
		Camera_t* camInfo = reinterpret_cast<Camera_t*>(param);
		pthread_mutex_lock(&capture_mutex);
		capture = cvCaptureFromAVI(camInfo->vid_filename);
		destroy_pending_task(START, param);
		apply_event(capture ? "success" : "aborted", NULL);
	}
	else if (transition == STREAMING_TO_READY)
		dispatcher->send_signal("controller", STOP_BTN_CLICKED, NULL);
	else if (transition == CONNECT_REQUESTED_TO_READY)
	{
		pthread_mutex_unlock(&capture_mutex);
		dispatcher->send_signal("controller", FAILURE, NULL);
	}	
	else if (transition == STREAMING_TO_STOP_REQUESTED)
	{
		pthread_mutex_lock(&capture_mutex);
		if (capture)
		{
			cvReleaseCapture(&capture);
			pthread_cond_signal(&capture_cond);
		}
		pthread_mutex_unlock(&capture_mutex);
		apply_event("aborted", NULL);
	} 
	else if (transition == CONNECT_REQUESTED_TO_STREAMING)
	{
		pthread_cond_signal(&capture_cond);
		pthread_mutex_unlock(&capture_mutex);
	}
}

