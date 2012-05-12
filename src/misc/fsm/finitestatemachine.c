#include "FiniteStateMachine.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct _FiniteStateMachine
{
	char* name;
	State* states;
	unsigned int stateCount;
	unsigned int capacity;
	State currentState;
	pthread_mutex_t mutex;
} _FiniteStateMachine;


FiniteStateMachine 
fsm_new(const char* name, const char* init_state_name)
{
	assert(name && init_state_name);
	State init_state = state_new(init_state_name);
	assert(init_state);
	FiniteStateMachine fsm = (FiniteStateMachine) malloc(sizeof(_FiniteStateMachine));
	assert(fsm);
	fsm->name = strdup(name);
	assert(fsm->name);
	fsm->currentState = init_state;
	fsm->stateCount = 1;
	fsm->capacity = 10;
	fsm->states = (State*)malloc(10*sizeof(State));
	assert(fsm->states);
	fsm->states[0] = init_state;
	pthread_mutex_init(&fsm->mutex, NULL);
	return fsm;
}

void              
fsm_free(FiniteStateMachine fsm)
{
	if (!fsm)
		return;

	/*free all the states*/
	int i;
	for (i=0;i<fsm->stateCount;i++)
		state_free(fsm->states[i]);

	free(fsm->name);
	pthread_mutex_destroy(&fsm->mutex);
	free(fsm);
}


void              
fsm_add_state(FiniteStateMachine fsm, const char* name)
{
	/*grow the buffer if required*/
	if (fsm->stateCount == fsm->capacity)
	{
		fsm->states = realloc (fsm->states, sizeof(State)*fsm->capacity*2);
		assert(fsm->states);
		fsm->capacity *=2 ;
	}

	fsm->states[fsm->stateCount] = state_new(name);
	fsm->stateCount++;
}

const Transition
fsm_link_states(const FiniteStateMachine fsm,
                              const char* transition_name,
                              const char* _pre_state,
                              const char* _post_state)
{
	assert (fsm && _pre_state && _post_state);
	State pre_state = NULL;
	State post_state = NULL;

	int i;
	for (i=0;i<fsm->stateCount;i++)
	{
		if (!strcmp(state_get_name(fsm->states[i]), _pre_state))
			pre_state = fsm->states[i];
		if (!strcmp(state_get_name(fsm->states[i]), _post_state))
			post_state = fsm->states[i];
		if (pre_state && post_state)
			break;
	}
	if (!pre_state || !post_state)
		return NULL;

	return transition_new(transition_name, pre_state, post_state);

}


const Transition
fsm_apply_transition(FiniteStateMachine fsm, const char* _transition)
{
	pthread_mutex_lock(&fsm->mutex);
	assert(fsm && _transition);
	const Transition transition = state_apply_transition(fsm->currentState, _transition);
	if (transition)
		fsm->currentState = transition_get_post_state(transition);
	pthread_mutex_unlock(&fsm->mutex);
	return transition;
}

const State
fsm_get_current_state(const FiniteStateMachine fsm)
{
	assert(fsm);
	return fsm->currentState;
}

const char*
fsm_get_current_state_name(const FiniteStateMachine fsm)
{
	assert(fsm);
	return state_get_name(fsm->currentState);
}
