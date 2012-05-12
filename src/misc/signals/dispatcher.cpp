#include "dispatcher.h"
#include <signalprocessor.h>
#include <iostream>
#include <string>
#include <SignalCode.h>

using namespace std;

/***********************************************
 * Constructor
 ***********************************************/ 
Dispatcher::Dispatcher()
{
	pthread_mutex_init(&state_mutex, NULL);
	pthread_mutex_init(&map_mutex, NULL);
}

/****************************************************************
 * It is assumed that register sp is called from a single thread. 
 * Normally the main thread and that's why there is no need to 
 * add any mutexes here
 ****************************************************************/ 
bool
Dispatcher::register_signal_processor(const std::string& name, SignalProcessor* sp, int reg_type)
{
	pthread_mutex_lock(&map_mutex);
	/*this name should not already exist*/
	if (_map.find(name)!= _map.end())
	{
		pthread_mutex_unlock(&map_mutex);
		return false;
	}

	_map[name] = sp;

	/*if the signal processor requests to be registered as a state recieving element ...*/
	if (reg_type & DISPATCHER_REG_TYPE_STATE_REPORT)
		state_requesters.push_back(sp);

 	pthread_mutex_unlock(&map_mutex);	
	return true;
}

/***************************************************************
 * This is the functin that is used by all SP in order to
 * transmit signals to one another
 ***************************************************************/ 
bool
Dispatcher::send_signal(const std::string& receiver, int code, void* param)
{
	pthread_mutex_lock(&map_mutex);
	std::map<std::string, SignalProcessor*>::iterator iter = _map.find(receiver);
	if (iter == _map.end())
	{
		pthread_mutex_unlock(&map_mutex);
		return false;
	}
	SignalProcessor* processor = iter->second;
	pthread_mutex_unlock(&map_mutex);
	processor->schedule_task(code, param);
	return true;
}

/****************************************************************
 * This is a synchronous call to set a parameter in a SP
 ****************************************************************/ 
void
Dispatcher::set_param(const std::string& receiver, const std::string& var, void* param)
{
	std::map<std::string, SignalProcessor*>::iterator iter = _map.find(receiver);
	if (iter == _map.end())
	{
		pthread_mutex_unlock(&map_mutex);
		return;
	}
	SignalProcessor* processor = iter->second;
	processor->set_param(var, param);
}

/****************************************************************
 * Destructor
 ****************************************************************/ 
Dispatcher::~Dispatcher()
{
	pthread_mutex_destroy(&state_mutex);
	pthread_mutex_destroy(&map_mutex);
}

void
/********************************************************
 * elements report if they change their states.
 * Dispatcher keeps a list of states and if an 
 * element's state changes then it would broadcast it
 * to all the elements registered with
 * DISPATCHER_REG_TYPE_STATE_REPORT
 ********************************************************/ 
Dispatcher::report_state(const char* _name, char* _state)
{
	pthread_mutex_lock(&state_mutex);
	string name = _name;
	string state = _state;
	free(_state);
	map<string, string>::iterator iter = states.find(name);
	if (iter == states.end() || (*iter).second.compare(state))
	{
		states[name] = state;
		pthread_mutex_lock(&map_mutex);
		for (vector<SignalProcessor*>::iterator iter = state_requesters.begin(); iter != state_requesters.end(); iter++)
			(*iter)->schedule_task(STATE_UPDATE, new map<string, string>(states));
		pthread_mutex_unlock(&map_mutex);
	}
	pthread_mutex_unlock(&state_mutex);
}
