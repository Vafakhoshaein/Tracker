#ifndef SIGNALPROCESSOR_H
#define SIGNALPROCESSOR_H

#include <iostream>
#include <queue>
#include <pthread.h>
#include <dispatcher.h>
extern "C"
{
#include <FiniteStateMachine.h>
}


typedef struct Signal {
	int code;
	void* param;
} Signal;

class  SignalProcessor
{
	private:
		std::queue<Signal> Q;
		pthread_mutex_t Q_mutex;
		pthread_cond_t Q_cond;
		pthread_t myThread;
		bool activated;
		char* name;
		static void* process_signal(void* _signal_processor);
		FiniteStateMachine fsm;

	protected:
		bool apply_event(const char* event, void* param);
		virtual void process(int code, void* param)=0;
		virtual void destroy_pending_task(int code, void* param)=0;
		virtual void handle_transition(Transition transition, void* param)=0;
		virtual void init_fsm(FiniteStateMachine fsm)=0;
		Dispatcher* dispatcher;

	public:
		SignalProcessor(const char* _name, Dispatcher* _dispatcher, int reg_type=DISPATCHER_REG_TYPE_NORMAL);
		void schedule_task(int code, void* param);
		pthread_t get_thread();
		virtual ~SignalProcessor();
		static void _init_fsm(SignalProcessor* sp);
		virtual void set_param(const std::string& var, void* param);
		int queue_size();
};

#endif // SIGNALPROCESSOR_H
