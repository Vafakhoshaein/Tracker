#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct _State
{
	char* name;
	Transition* transitions;
	unsigned int capacity;      /*transition list capacity*/
	unsigned int transitionCount;
} _State;

State
state_new(const char* name)
{
	_State* state = (_State*)malloc(sizeof(_State));
	assert(state);
	state->name = strdup(name);
	assert(state->name);
	state->transitionCount = 0;
	state->capacity = 10;
	state->transitions = malloc(sizeof(Transition) * 10);
	assert(state->transitions);
	memset(state->transitions, 0, sizeof(Transition) * 10);
	return state;
}


void
state_free(State state)
{
	if (!state)
		return;
	free(state->name);
	int i;
	for (i=0;i<state->transitionCount;i++)
		transition_free(state->transitions[i]);
	free(state->transitions);
	free(state);
}

const char*
state_get_name(const State state)
{
	return state->name;
}


void
state_add_transition(const State state, const Transition transition)
{
	assert (state && transition);
	if (state->transitionCount == state->capacity)
	{
		state->transitions = realloc(state->transitions, state->capacity*2);
		assert(state->transitions);
		state->capacity *=2;
	}
	state->transitions[state->transitionCount] = transition;
	state->transitionCount++;
}

const Transition
state_apply_transition(const State state, const char* _transition)
{
	assert(state && _transition);
	int i;
	for (i=0;i<state->transitionCount;i++)
		if (!strcmp(transition_get_name(state->transitions[i]), _transition))
			return state->transitions[i];

	return NULL;
}

