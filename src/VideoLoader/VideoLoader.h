#ifndef VIDEO_LOADER_H
#define VIDEO_LOADER_H

#include <QImage>

extern "C"
{
#include <FiniteStateMachine.h>
}
#include <signalprocessor.h>
#include <tracker_types.h>
#include <dispatcher.h>
#include <cv.h>
#include "highgui.h"

class VideoLoader : public SignalProcessor
{
	public:
		VideoLoader(Dispatcher* _dispatcher);

	protected:
		void process(int code, void *param);
		void destroy_pending_task(int code, void *param);
		void init_fsm(FiniteStateMachine fsm);
		void handle_transition(Transition transition, void* param);

	private:
		Transition READY_TO_CONNECT_REQUESTED, 
			   CONNECT_REQUESTED_TO_STREAMING, 
			   CONNECT_REQUESTED_TO_READY, 
			   STREAMING_TO_STOP_REQUESTED, 
			   STREAMING_TO_READY, 
			   STOP_REQUESTED_TO_READY;
		CvCapture *capture;
		static void* streamingThreadFunc(void* );
		pthread_t streamingThread;

		pthread_mutex_t capture_mutex;
		pthread_cond_t capture_cond;
};




#endif 
