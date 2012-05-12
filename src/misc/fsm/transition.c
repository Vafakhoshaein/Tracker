#include "transition.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct _Transition
{
	char* name;
	State pre_state;
	State post_state;
} _Transition;


Transition
transition_new(const char* name, const State pre_state, const State post_state)
{
	assert(pre_state && post_state);
	Transition transition = (Transition)malloc(sizeof(_Transition));
	assert(transition);
	transition->name = strdup(name);
	assert(transition->name);
	transition->pre_state = pre_state;
	transition->post_state = post_state;
	state_add_transition(pre_state, transition);
	return transition;
}

void
transition_free(const Transition transition)
{
	if (!transition)
		return;
	free(transition->name);
	free(transition);
}

const char*
transition_get_name(const Transition transition)
{
	assert(transition);
	return transition->name;
}

State
transition_get_pre_state(const Transition transition)
{
	assert(transition);
	return transition->pre_state;
}

State
transition_get_post_state(const Transition transition)
{
	assert(transition);
	return transition->post_state;
}

