#ifndef TRANSITION_H
#define TRANSITION_H

typedef struct _Transition* Transition;
#include <state.h>

Transition transition_new(const char* name, const State head, const State tail);
void       transition_free(const Transition transition);

/*getters*/
const char* transition_get_name(const Transition transition);
State       transition_get_pre_state(const Transition transition);
State       transition_get_post_state(const Transition transition);


#endif // TRANSITION_H
