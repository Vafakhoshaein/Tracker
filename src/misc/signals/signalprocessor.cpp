#include "signalprocessor.h"
#include "stdio.h"
#include "string.h"
extern "C"
{
#include "FiniteStateMachine.h"
}

SignalProcessor::SignalProcessor(const char* _name, Dispatcher* _dispatcher, int reg_type):activated(true), 
											   dispatcher(_dispatcher)
{
	name = strdup(_name);
	pthread_mutex_init(&Q_mutex, NULL);
	pthread_cond_init(&Q_cond, NULL);
	pthread_create(&myThread, NULL, SignalProcessor::process_signal, this);
	dispatcher->register_signal_processor(name, this, reg_type);
}

void
SignalProcessor::_init_fsm(SignalProcessor* sp)
{
	sp->fsm = fsm_new(sp->name, "ready");
	sp->init_fsm(sp->fsm);
	sp->dispatcher->report_state(sp->name, strdup(fsm_get_current_state_name(sp->fsm)));
}

SignalProcessor::~SignalProcessor()
{
	pthread_mutex_destroy(&Q_mutex);
	pthread_cond_destroy(&Q_cond);
	free(name);
	fsm_free(fsm);
}

void*
SignalProcessor::process_signal(void* _signal_processor)
{
	SignalProcessor* signal_processor = reinterpret_cast<SignalProcessor*>(_signal_processor);
	while (signal_processor->activated)
	{
		pthread_mutex_lock(&signal_processor->Q_mutex);
		/*wait until a new task is scheduled if no task is scheduled*/
		if (signal_processor->Q.empty())
			pthread_cond_wait(&signal_processor->Q_cond, &signal_processor->Q_mutex);

		Signal currentSignal = signal_processor->Q.front();
		signal_processor->Q.pop();
		pthread_mutex_unlock(&signal_processor->Q_mutex);

		switch (currentSignal.code)
		{
			/*designated code to exit*/
			case 0:
				{
					pthread_mutex_lock(&signal_processor->Q_mutex);
					signal_processor->activated = false;

					/*destroy all the pending signals*/
					while (!signal_processor->Q.empty())
					{
						currentSignal = signal_processor->Q.front();
						signal_processor->Q.pop();
						signal_processor->destroy_pending_task(currentSignal.code, currentSignal.param);
					}
					pthread_mutex_unlock(&signal_processor->Q_mutex);
				}
					break;
			default:
				signal_processor->process(currentSignal.code, currentSignal.param);
				break;
		}
	}
	return 0;
}


void
SignalProcessor::schedule_task(int code, void* param)
{
	pthread_mutex_lock(&Q_mutex);
	if (activated)
	{
		Signal newSignal;
		newSignal.code = code;
		newSignal.param = param;
		Q.push(newSignal);
		pthread_cond_signal(&Q_cond);
	}
	else
		/*this signal was late and must be destroyed*/
		destroy_pending_task(code, param);
	pthread_mutex_unlock(&Q_mutex);
}


pthread_t
SignalProcessor::get_thread()
{
	return myThread;
}

void
SignalProcessor::set_param(const std::string& var, void* param)
{
}


bool 
SignalProcessor::apply_event(const char* event, void* param)
{
	Transition transition = fsm_apply_transition(fsm, event);	
	if (transition)
	{
		handle_transition(transition, param);
		dispatcher->report_state(name, strdup(fsm_get_current_state_name(fsm)));
		return true;
	}
	return false;
}

int
SignalProcessor::queue_size()
{
	return Q.size(); 
}
