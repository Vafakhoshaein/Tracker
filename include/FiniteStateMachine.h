#ifndef FiniteStateMachine_H
#define FiniteStateMachine_H

#include <state.h>

typedef struct _FiniteStateMachine* FiniteStateMachine;

/*constructor/destructors*/
FiniteStateMachine fsm_new(const char* name, const char* init_state_name);
void              fsm_free(FiniteStateMachine fsm);

/*adding/linking*/
void              fsm_add_state(FiniteStateMachine fsm, const char* name);
const Transition  fsm_link_states(FiniteStateMachine fsm, const char* transition_name, const char* pre_state, const char* post_state);

/*operations*/
const Transition  fsm_apply_transition(const FiniteStateMachine fsm, const char* transition);
const char*       fsm_get_current_state_name(const FiniteStateMachine fsm);
const State       fsm_get_current_state(const FiniteStateMachine fsm);


#endif // FiniteStateMachine_H
