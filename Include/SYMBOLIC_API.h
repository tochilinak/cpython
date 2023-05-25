#ifndef CPYTHON_SYMBOLIC_API_H
#define CPYTHON_SYMBOLIC_API_H

typedef PyObject *(*symbolic_handler_callable)(int event_type, int event_id, int nargs, PyObject *const *args, void *param);

// event types
#define SYM_EVENT_TYPE_STACK   0
#define SYM_EVENT_TYPE_NOTIFY  1

// event ids
#define SYM_EVENT_ID_CREATE_LIST  0
#define SYM_EVENT_ID_CONST        1
#define SYM_EVENT_ID_INSTRUCTION  2
#define SYM_EVENT_ID_FORK         3
#define SYM_EVENT_ID_FORK_RESULT  4

#endif //CPYTHON_SYMBOLIC_API_H
