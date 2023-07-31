#ifndef CPYTHON_SYMBOLIC_API_H
#define CPYTHON_SYMBOLIC_API_H

typedef PyObject *(*symbolic_handler_callable)(int event_type, int event_id, int nargs, PyObject *const *args, void *param);

// event types
#define SYM_EVENT_TYPE_STACK   0
#define SYM_EVENT_TYPE_NOTIFY  1
#define SYM_EVENT_TYPE_METHOD  2

// event ids
#define SYM_EVENT_ID_CREATE_LIST          0
#define SYM_EVENT_ID_CONST                1
#define SYM_EVENT_ID_INSTRUCTION          2
#define SYM_EVENT_ID_FORK                 3
#define SYM_EVENT_ID_FORK_RESULT          4
#define SYM_EVENT_ID_INT_GT               5
#define SYM_EVENT_ID_INT_LT               6
#define SYM_EVENT_ID_INT_EQ               7
#define SYM_EVENT_ID_INT_NE               8
#define SYM_EVENT_ID_INT_GE               9
#define SYM_EVENT_ID_INT_LE               10
#define SYM_EVENT_ID_INT_ADD              11
#define SYM_EVENT_ID_INT_SUB              12
#define SYM_EVENT_ID_INT_NEG              13
#define SYM_EVENT_ID_INT_MULT             14
#define SYM_EVENT_ID_INT_REM              15
#define SYM_EVENT_ID_INT_FLOORDIV         16
#define SYM_EVENT_ID_INT_POW              17
#define SYM_EVENT_ID_PYTHON_FUNCTION_CALL 18
#define SYM_EVENT_ID_RETURN               19
#define SYM_EVENT_ID_LIST_GET_ITEM        20
#define SYM_EVENT_ID_LIST_SET_ITEM        21
#define SYM_EVENT_ID_LIST_EXTEND          22
#define SYM_EVENT_ID_LIST_APPEND          23
#define SYM_EVENT_ID_VIRTUAL_RICHCMP      24
#define SYM_EVENT_ID_VIRTUAL_MP_SUBSCRIPT 25
#define SYM_EVENT_ID_NB_BOOL              26
#define SYM_EVENT_ID_NB_INT               27
#define SYM_EVENT_ID_MP_SUBSCRIPT         28
#define SYM_EVENT_ID_TP_RICHCMP           29

PyAPI_DATA(void*) virtual_tp_richcompare;
PyAPI_DATA(void*) virtual_mp_subscript;

#endif //CPYTHON_SYMBOLIC_API_H
