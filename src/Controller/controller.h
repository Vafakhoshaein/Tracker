#include <signalprocessor.h>
#include <dispatcher.h>
#include <mainwindow.h>
#include <highgui.h>
#include <map>
#include <string>

using namespace std;


#ifndef CONTROLLER_H 
#define CONTROLLER_H

class Controller : public SignalProcessor
{
	private:
		bool b_using_ipcam;
		MainWindow* mainWindow;
		cv::VideoWriter* videoWriter;
		Transition READY_TO_START_REQUESTED,
			   START_REQUESTED_TO_STARTED,
			   STARTED_TO_EXIT_REQUESTED,
			   START_REQUESTED_TO_EXIT_REQUESTED,
			   START_REQUESTED_TO_STOP_REQUESTED,
			   STARTED_TO_RETRY,
			   RETRY_TO_RETRY,
			   RETRY_TO_START_REQUESTED, 
			   RETRY_TO_EXIT_REQUESTED, 
			   RETRY_TO_STOP_REQUESTED,
			   STARTED_TO_STOP_REQUESTED,
			   STARTED_TO_STARTED,
			   STOP_REQUESTED_TO_READY,
			   STOP_REQUESTED_TO_EXIT_REQUESTED,
			   EXIT_REQUESTED_TO_EXIT,
			   READY_TO_EXIT;

		void confirm_external_states(map<string, string>& states);

	protected:
		void process(int code, void *param);
		void destroy_pending_task(int code, void *param);
		void init_fsm(FiniteStateMachine fsm);
		void handle_transition(Transition transition, void* param);

	public:
		Controller(Dispatcher* _dispatcher, MainWindow* _mainWindow);
		~Controller();
};

#endif // CONTROLLER_H 

