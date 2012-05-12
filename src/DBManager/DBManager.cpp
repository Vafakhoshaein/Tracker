#include <DBManager.h>
#include <SignalCode.h>
#include <common.h>
#include <stdio.h>
#include <glib.h>

DBManager::DBManager(Dispatcher* _dispatcher):mysql(0),
					      SignalProcessor("dbmanager", _dispatcher)
{
	_init_fsm(this);
}

void 
DBManager::init_fsm(FiniteStateMachine fsm)
{
	fsm_add_state(fsm, "try-connect");
	fsm_add_state(fsm, "connected");
	fsm_add_state(fsm, "try-insert");

	READY_TO_TRY_CONNECT = fsm_link_states(fsm, "connect", "ready", "try-connect");
	TRY_CONNECT_TO_CONNECTED = fsm_link_states(fsm, "success", "try-connect", "connected"); 
	TRY_CONNECT_TO_READY = fsm_link_states(fsm, "fail", "try-connect", "ready");
	CONNECTED_TO_READY = fsm_link_states(fsm, "stop", "connected", "ready");
	CONNECTED_TO_TRY_INSERT	= fsm_link_states(fsm, "insert", "connected", "try-insert");
	TRY_INSERT_TO_CONNECTED	= fsm_link_states(fsm, "inserted", "try-insert", "connected");
	TRY_INSERT_TO_READY= fsm_link_states(fsm, "fail", "try-insert", "ready");
}

void
DBManager::process(int code, void* param)
{
	switch (code)
	{
		case START:
			{
				if (!apply_event("connect", param))
					destroy_pending_task(START, param);
				break;
			}
		case STOP:
			{
				apply_event("stop", NULL);
				break;
			}
		case DB_INSERT:
			{
				apply_event("insert", param);
				break;
			}
		case DB_INSERT_VIDEO_SEQUENCE:
			{
				gchar* str = reinterpret_cast<gchar*> (param);
				gchar* statement = g_strdup_printf 
					("insert into video_file_reference " \
					 "(filename, startTime, startFrame, endTime, endFrame, session_id) " \
					 "values (%s, %lld)", str, session_id);
				g_free(str);
				apply_event("insert", statement);
				break;
			}
		case DB_INSERT_TARGET:
			{
				vector<TargetPtr>* targets = reinterpret_cast<vector<TargetPtr>*>(param);
				gchar* statement = g_strdup("start transaction");
				apply_event("insert", statement);

				for (vector<TargetPtr>::iterator iter = targets->begin(); iter != targets->end(); iter++)
					insert_target(*iter);
				targets->clear();
				delete targets;

				statement = g_strdup("commit");
				apply_event("insert", statement);

				break;

		}
	}
}

void
DBManager::insert_node(BlobFeature blob, int trace_id, int node_id)
{
	const int* ch = getColorHistogram(blob);
	CvRect r = rect(blob);
	gchar* statement = g_strdup_printf("insert into trace_node("\
	"xcor,ycor,height,width,node_id,trace_id,session_id)"
	"values (%d, %d, %d, %d, %d, %d, %lld)",
	r.x, r.y, r.height, r.width, node_id, trace_id, session_id);

	apply_event("insert", statement);
}

void
DBManager::insert_trace(TracePtr trace, int target_id)
{
	int trace_id = trace->get_id();
	gchar* statement = g_strdup_printf(
			"insert into trace (trace_id, first_frame, target_id, session_id)" \
			"values (%d, %d, %d, %lld)",\
			trace_id, trace->get_initial_frame(), target_id, session_id);
	apply_event("insert", statement);
	int node_id = 0;
	for (vector<BlobFeature>::iterator iter = trace->nodes.begin(); iter != trace->nodes.end(); iter++, node_id++)
		insert_node(*iter, trace_id, node_id);
}

void
DBManager::insert_target(TargetPtr target)
{
	int target_id = target->get_id();
	gchar* statement = g_strdup_printf(
			"insert into target (target_id, session_id) values(%d, %lld)", 
			target_id,
			session_id);
	apply_event("insert", statement);
	vector<TracePtr>& traces = target->getTraces();
	for (vector<TracePtr>::iterator iter = traces.begin(); iter != traces.end(); iter++)
		insert_trace(*iter, target_id);
}

void
DBManager::destroy_pending_task(int code, void *param)
{
	switch (code)
	{
		case START:
			{
				Database_t* dbinfo = reinterpret_cast<Database_t*>(param);
				free(dbinfo->db_database);
				free(dbinfo->db_hostname);
				free(dbinfo->db_password);
				free(dbinfo->db_username);
				delete dbinfo;
				break;
			}
		case DB_INSERT:
			{
				gchar* str = reinterpret_cast<gchar*> (param);
				g_free(str);
				break;
			}
		case DB_INSERT_VIDEO_SEQUENCE:
			{
				gchar* str = reinterpret_cast<gchar*> (param);
				g_free(str);
				break;
			}
		case DB_INSERT_TARGET:
			{
				vector<TargetPtr>* targets = reinterpret_cast <vector<TargetPtr>*>(param);
				delete targets;
				break;
			}
	}
}

void 
DBManager::handle_transition(Transition transition, void* param)
{
	if (!transition)
		return;

	if (transition == READY_TO_TRY_CONNECT)
	{
		Database_t* dbinfo = reinterpret_cast<Database_t*>(param);
		mysql = mysql_init(NULL);
		if (!mysql)
		{
			apply_event("fail", NULL);
		}
		else
		{
			if (!mysql_real_connect(mysql, dbinfo->db_hostname, dbinfo->db_username, 
						dbinfo->db_password, dbinfo->db_database,dbinfo->db_port,
						NULL, 0))
				apply_event("fail", NULL);
			else
				apply_event("success", NULL);
		}
		destroy_pending_task(START, param);
	}
	else if (transition == TRY_CONNECT_TO_CONNECTED)
	{
		mysql_query(mysql, "insert into session (name, start_time) values(\"test\", now());");
		session_id = mysql_insert_id(mysql);
	}
	else if (transition == CONNECTED_TO_READY)
	{
		gchar* statement = g_strdup_printf("update session set session.end_time=now() where session.session_id=%lld", session_id);
		mysql_query(mysql, statement);
		g_free(statement);
		mysql_close(mysql);
		mysql = NULL;
	}	
	else if (transition == CONNECTED_TO_TRY_INSERT)
	{
		gchar* statement = reinterpret_cast<char*> (param);
		struct timeval t1, t2;
		int retVal = mysql_query(mysql, statement);
		g_free(statement);
		if (!retVal)
			apply_event("inserted", NULL);
		else
			apply_event("fail", NULL);
	} 
	else if (transition == TRY_INSERT_TO_CONNECTED)
	{
	}
	else if (transition ==TRY_INSERT_TO_READY)
	{
		gchar* statement = g_strdup_printf("update session set session.end_time=now() where session.session_id=%lld", session_id);
		mysql_query(mysql, statement);
		g_free(statement);
		mysql_close(mysql);
		mysql = NULL;
		dispatcher->send_signal("controller", FAILURE, NULL);
	}
	else if (transition == TRY_CONNECT_TO_READY)
	{
		dispatcher->send_signal("controller", FAILURE, NULL);
		if (mysql)
			mysql_close(mysql);
		mysql = NULL;
	}
}

