#ifndef STATE_H
#define STATE_H

typedef struct _State* State;
#include <transition.h>

State state_new(const char* name);
void state_free(State state);



/*getters*/
const char* state_get_name(const State state);

/*operations*/
void state_add_transition(const State state, const Transition transition);
const Transition state_apply_transition(const State state, const char* transition);

#endif // STATE_H
