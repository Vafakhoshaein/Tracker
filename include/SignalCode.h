#ifndef SIGNALCODE_H
#define SIGNALCODE_H

/*CORE-SIGNALS*/
#define TERMINATION 			0
#define STATE_UPDATE 			1

/*APPLICATIN SIGNALS*/
#define STOP_BTN_CLICKED        	2
#define EXIT_BTN_CLICKED        	3
#define IPCAM_STATE_UPDATE      	4
#define START                   	5
#define ABORTED                 	6
#define CONNECTED               	7
#define STOP                    	8
#define FAILURE                 	9
#define NEW_IMAGE               	10
#define PAIR_IMAGE              	11
#define TRACKER_STATE_UPDATE    	12
#define TURN_ON_BOUNDING_BOX    	13
#define TURN_OFF_BOUNDING_BOX   	14
#define TURN_ON_DRAW_TRACES     	15
#define TURN_OFF_DRAW_TRACES    	16
#define TURN_ON_DRAW_TARGETS    	17
#define TURN_OFF_DRAW_TARGETS   	18
#define VIDEO_LOADER_STATE_UPDATE 	19
#define DB_MANAGER_STATE_UPDATE 	20
#define DB_INSERT 			21
#define DB_INSERT_VIDEO_SEQUENCE 	22
#define DB_INSERT_TARGET 		23
#define START_BTN_CLICKED        	24	

#endif // SIGNALCODE_H
