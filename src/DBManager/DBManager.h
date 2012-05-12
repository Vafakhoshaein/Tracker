#ifndef DB_MANAGER_H 
#define DB_MANAGER_H

extern "C"
{
#include <FiniteStateMachine.h>
}
#include <signalprocessor.h>
#include <tracker_types.h>
#include <dispatcher.h>
#include <mysql/mysql.h>
#include <trace.h>

class DBManager : public SignalProcessor
{
	public:
		DBManager(Dispatcher* _dispatcher);

	protected:
		void process(int code, void *param);
		void destroy_pending_task(int code, void *param);
		void init_fsm(FiniteStateMachine fsm);
		void handle_transition(Transition transition, void* param);

	private:
		Transition READY_TO_TRY_CONNECT,
			   TRY_CONNECT_TO_READY,
			   TRY_CONNECT_TO_CONNECTED,
			   CONNECTED_TO_READY,
			   CONNECTED_TO_TRY_INSERT,
			   TRY_INSERT_TO_CONNECTED,
			   TRY_INSERT_TO_READY;
		MYSQL* mysql;
		my_ulonglong session_id;
		void insert_target(TargetPtr target);
		void insert_trace(TracePtr trace, int target_id);
		void insert_node(BlobFeature blob, int trace_id, int node_id);
};




#endif 
