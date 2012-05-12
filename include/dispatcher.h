#include <iostream>
#include <map>
#include <string>
#include <vector>

extern "C"
{
#include <FiniteStateMachine.h>
}


#ifndef DISPATCHER_H
#define DISPATCHER_H

/*foreward declaration*/
class SignalProcessor;

/**************************************************************************************** 
 * Registration types
 * When a SP registers with a dispatcher, it will report what it needs to be informed of:
 * For example: everytime an element state changes in the ring, the elements registered
 * as STATE_REPORT will receive a signal informing them of the states
 ****************************************************************************************/
#define DISPATCHER_REG_TYPE_NORMAL 	 	0	
#define DISPATCHER_REG_TYPE_STATE_REPORT 	1

class Dispatcher
{
	private:
		std::map<std::string, SignalProcessor*> _map;
		std::map<std::string, std::string> states;
		std::vector<SignalProcessor*> state_requesters;
		pthread_mutex_t state_mutex;
		pthread_mutex_t map_mutex;
	public:
		Dispatcher();
		~Dispatcher();
		bool register_signal_processor(const std::string& name, SignalProcessor* sp, int reg_type);
		bool send_signal(const std::string& receiver, int code, void* param);
		void set_param(const std::string& receiver, const std::string& var, void* param);
		void report_state(const char* name, char* state);
};

#endif // DISPATCHER_H
