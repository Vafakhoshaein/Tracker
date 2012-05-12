#include "controller.h"
#include <assert.h>
#include <tracker_types.h>
#include <SignalCode.h>
#include <stdio.h>
#include <QImage>
#include <QDebug>
#include <common.h>
#include <dispatcher.h>
#include <trace.h>


Controller::Controller(Dispatcher* _dispatcher, 
 		       MainWindow* _mainWindow):SignalProcessor("controller", _dispatcher, DISPATCHER_REG_TYPE_STATE_REPORT),
			 			mainWindow(_mainWindow),
						videoWriter(0)
{
	_init_fsm(this);
}


void
Controller::init_fsm(FiniteStateMachine fsm)
{
	fsm_add_state(fsm, "start-requested");
	fsm_add_state(fsm, "started");
	fsm_add_state(fsm, "exit-requested");
	fsm_add_state(fsm, "stop-requested");
	fsm_add_state(fsm, "retry");
	fsm_add_state(fsm, "exit");
	READY_TO_START_REQUESTED = fsm_link_states(fsm, "start-btn-clicked", "ready", "start-requested");
	START_REQUESTED_TO_STARTED = fsm_link_states(fsm, "everything-started", "start-requested", "started");
	STARTED_TO_EXIT_REQUESTED =  fsm_link_states(fsm, "exit-btn-clicked", "started", "exit-requested");
	STARTED_TO_STARTED = fsm_link_states(fsm, "new-image", "started", "started");
	START_REQUESTED_TO_EXIT_REQUESTED = fsm_link_states(fsm, "exit-btn-clicked", "start-requested", "exit-requested");
	START_REQUESTED_TO_STOP_REQUESTED = fsm_link_states(fsm, "failed", "start-requested", "stop-requested");

	STARTED_TO_RETRY = fsm_link_states(fsm, "failed", "started", "retry");
	RETRY_TO_RETRY = fsm_link_states(fsm, "everything-stopped", "retry", "retry");
	RETRY_TO_START_REQUESTED = fsm_link_states(fsm, "start-btn-clicked", "retry", "start-requested");
	RETRY_TO_EXIT_REQUESTED = fsm_link_states(fsm, "exit-btn-clicked", "retry", "exit-requested");
	RETRY_TO_STOP_REQUESTED = fsm_link_states(fsm, "stop-btn-clicked", "retry", "stop-requested");

	STARTED_TO_STOP_REQUESTED = fsm_link_states(fsm, "stop-btn-clicked", "started", "stop-requested");
	STOP_REQUESTED_TO_READY = fsm_link_states(fsm, "everything-stopped", "stop-requested", "ready");
	STOP_REQUESTED_TO_EXIT_REQUESTED = fsm_link_states(fsm, "exit-btn-clicked", "stop-requested", "exit-requested");
	EXIT_REQUESTED_TO_EXIT = fsm_link_states(fsm, "everything-stopped", "exit-requested", "exit");
	READY_TO_EXIT = fsm_link_states(fsm, "exit-btn-clicked", "ready", "exit");

	mainWindow->stateReport(strdup(fsm_get_current_state_name(fsm)));
}

Controller::~Controller()
{
}

void 
Controller::handle_transition(Transition transition, void* param)
{
	if (transition == READY_TO_START_REQUESTED || transition == RETRY_TO_START_REQUESTED)
	{
		std::pair<Database_t*, Camera_t*>* temp_pair = reinterpret_cast<std::pair<Database_t*, Camera_t*>*>(param);
		if (temp_pair->second->cam_brand)
		{
			dispatcher->set_param("tracker", "prefix", temp_pair->second->vid_dir);
			dispatcher->send_signal("ipcameracapture", START, temp_pair->second);
			b_using_ipcam = true;
		}
		else
		{
			bool recorder_switch = false;
			dispatcher->set_param("tracker", "recorder-switch", &recorder_switch); 
			dispatcher->send_signal("videoloader", START, temp_pair->second);
			b_using_ipcam = false;
		}

		dispatcher->send_signal("tracker", START, NULL);
		dispatcher->send_signal("dbmanager", START, temp_pair->first);

		delete temp_pair;
	}
	else if (transition == READY_TO_EXIT || transition == EXIT_REQUESTED_TO_EXIT)
	{
		dispatcher->send_signal("controller", 0, NULL);
		dispatcher->send_signal("ipcameracapture", 0, NULL);
		dispatcher->send_signal("tracker", 0, NULL);
		dispatcher->send_signal("videoloader", 0, NULL);
		dispatcher->send_signal("dbmanager", 0, NULL);
	}
	else if (transition == STARTED_TO_RETRY ||
			transition == STARTED_TO_STOP_REQUESTED ||
			transition == START_REQUESTED_TO_STOP_REQUESTED ||
			transition == START_REQUESTED_TO_EXIT_REQUESTED ||
			transition == STARTED_TO_EXIT_REQUESTED)
	{
		dispatcher->send_signal("ipcameracapture", STOP, NULL);
		dispatcher->send_signal("tracker", STOP, NULL);
		dispatcher->send_signal("videoloader", STOP, NULL);
	}
	else if (transition == STARTED_TO_STARTED)
		dispatcher->send_signal("tracker", NEW_IMAGE, param);
	else if (transition == RETRY_TO_RETRY)
	{
		sleep (10);
		mainWindow->on_startBtn_clicked();
	}
	else if (transition == START_REQUESTED_TO_STARTED && mainWindow->isRecordVideoChkBoxChecked())
	{
		videoWriter = new cv::VideoWriter("output.avi", CV_FOURCC('M','J','P','G'), 30, cvSize(640, 480), true);
	}
	else if ((transition == STOP_REQUESTED_TO_READY || transition == EXIT_REQUESTED_TO_EXIT) && videoWriter)
	{
		delete videoWriter;
		videoWriter = NULL;
	}

	if (transition != STARTED_TO_STARTED)
		mainWindow->stateReport(strdup(state_get_name(transition_get_post_state(transition))));
}


void 
Controller::confirm_external_states(map<string, string>& states)
{
	string ipcam_state = states["ipcameracapture"];
	string tracker_state = states["tracker"];
	string dbmanager_state = states["dbmanager"];
	string video_loader_state = states["videoloader"];

	if (ipcam_state.empty() || tracker_state.empty() || video_loader_state.empty() || dbmanager_state.empty())
		return;

	if (!ipcam_state.compare("ready") &&
	    !tracker_state.compare("ready") &&
	    !video_loader_state.compare("ready") &&
	    !dbmanager_state.compare("ready"))
	{
		apply_event("everything-stopped", NULL);
	}
	else if ((!ipcam_state.compare("streaming") || !video_loader_state.compare("streaming")) &&
		 (!tracker_state.compare("not-recording") || !tracker_state.compare("recording")) &&
		 (!dbmanager_state.compare("connected") || !dbmanager_state.compare("try-insert")))
	{
		apply_event("everything-started", NULL);
	}
	else if(!ipcam_state.compare("ready") &&
	    !tracker_state.compare("ready") &&
	    !video_loader_state.compare("ready") &&
	    dbmanager_state.compare("ready"))
	{
		/*all modules have stopped, it's time to stop dbmanager*/
		dispatcher->send_signal("dbmanager", STOP, NULL);
	}
}

void 
Controller::process(int code, void *param)
{
	Transition t;
	switch(code)
	{
		case START_BTN_CLICKED: /*fired from GUI*/
			{
				if (!apply_event("start-btn-clicked", param))
					destroy_pending_task(code, param);
				break;
			}
		case STOP_BTN_CLICKED: /*fired from GUI*/
			{
				apply_event("stop-btn-clicked", NULL);
				break;
			}
		case EXIT_BTN_CLICKED: /*fired from GUI*/
			{
				apply_event("exit-btn-clicked", NULL);
				break;
			}
		case FAILURE: /*may be fired from any component*/
			{
				apply_event("failed", NULL);
				break;
			}
		case NEW_IMAGE: /*fired from ipcameracapture*/
			{
				if (!apply_event("new-image", param))
					destroy_pending_task(code, param);
				break;
			}
		case PAIR_IMAGE:  /*fired from tracker - redirect to GUI*/
			{
				std::pair<QImage*, QImage*>* temp_pair = reinterpret_cast<std::pair<QImage*, QImage*>*>(param);
				if (videoWriter)
				{	
					cv::Mat* img = QImage2IplImage(temp_pair->first);
					*videoWriter << *img;
					delete img;
				}
				mainWindow->updateUI_ImagePair(temp_pair->first, temp_pair->second);
				delete temp_pair;
				break;
			}
		case STATE_UPDATE:
			{
				std::map<string, string>* report = reinterpret_cast<std::map<string, string>*>(param);
				confirm_external_states(*report);
				delete report;
				break;
			}
		case DB_INSERT_TARGET:
			{
				if (mainWindow->isLogTargetsChkBoxChecked())
				{
					QString str = mainWindow->getLogTargetFilename();
					QByteArray array = str.toLocal8Bit();
					const char* _str = array.data();
					FILE* fp = fopen (_str, "a+");
					if (fp)
					{
						vector<TargetPtr>* targets = reinterpret_cast<vector<TargetPtr>*>(param);
						for (vector<TargetPtr>::iterator iter = targets->begin(); iter != targets->end(); iter++)
						{
							vector<TracePtr> traces = (*iter)->getTraces();
							for (vector<TracePtr>::iterator traceIter = traces.begin();
									traceIter != traces.end();
									traceIter++)
							{
								TracePtr trace = *traceIter;
								int target_id = (*iter)->get_id();
								int frame_number = trace->get_initial_frame();
								for (vector<BlobFeature>::iterator blobIter = trace->nodes.begin();
										blobIter != trace->nodes.end();
										blobIter++)
								{
									BlobFeature blob = *blobIter;
									CvRect r = rect(blob);
									fprintf (fp, "%d[%d]:%d,%d\n", frame_number, target_id, r.x, r.y); 
									frame_number++;
								}

							}
						}
						fclose(fp);
					}
					else
					{
						printf("Warning: %s log file could not be opened for writing\n", _str);
					}
				}
				dispatcher->send_signal("dbmanager", DB_INSERT_TARGET, param);
				break;
			}
		case DB_INSERT_VIDEO_SEQUENCE:
			{
				dispatcher->send_signal("dbmanager", DB_INSERT_VIDEO_SEQUENCE, param);
				break;
			}
	}
}

void 
Controller::destroy_pending_task(int code, void *param)
{
	switch(code)
	{
		case START_BTN_CLICKED:
			{
				std::pair<Database_t*, Camera_t*>* temp_pair = reinterpret_cast<std::pair<Database_t*, Camera_t*>*>(param);
				free(temp_pair->first->db_database);
				free(temp_pair->first->db_hostname);
				free(temp_pair->first->db_password);
				free(temp_pair->first->db_username);
				free(temp_pair->second->vid_dir);
				if (temp_pair->second->cam_brand)
				{
					free(temp_pair->second->cam_brand);
					free(temp_pair->second->cam_password);
					free(temp_pair->second->cam_uri);
					free(temp_pair->second->cam_username);
				}
				else
					free(temp_pair->second->vid_filename);
				delete temp_pair;
				break;
			}
		case NEW_IMAGE:
			{
				QImage* img = reinterpret_cast<QImage*>(param);
				delete img;
			}
	}
}

