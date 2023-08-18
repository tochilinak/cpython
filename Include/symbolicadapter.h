#ifndef CPYTHON_SYMBOLICADAPTER_H
#define CPYTHON_SYMBOLICADAPTER_H

#include "Python.h"

#define SymbolicAdapterTypeName "ibmviqhlye.___SymbolicAdapter___ibmviqhlye"

typedef PyObject *(*unary_handler)(void *, PyObject *o);
typedef PyObject *(*binary_handler)(void *, PyObject *left, PyObject *right);
typedef PyObject *(*ternary_handler)(void *, PyObject *o1, PyObject *o2, PyObject *o3);
typedef int (*ternary_notify)(void *, PyObject *o1, PyObject *o2, PyObject *o3);

typedef struct {
    PyObject_HEAD
    void *handler_param;
    PyObject *ready_wrapper_types;
    int ignore;
    void *inside_wrapper_tp_call;
    int (*instruction)(void *, PyFrameObject *frame);
    int (*fork_notify)(void *, PyObject *on);
    int (*fork_result)(void *, PyObject *on, int result);
    int (*function_call)(void *, PyObject *code);
    int (*function_return)(void *, PyObject *code);
    PyObject *(*load_const)(void *, PyObject *obj);
    PyObject *(*create_list)(void *, PyObject **elems);
    PyObject *(*gt_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*lt_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*eq_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*ne_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*le_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*ge_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*add_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*sub_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*mul_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*div_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*rem_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*pow_long)(void *, PyObject *base, PyObject *pow, PyObject *mod);
    PyObject *(*bool_and)(void *, PyObject *, PyObject *);
    PyObject *(*list_get_item)(void *, PyObject *storage, PyObject *index);
    int (*list_set_item)(void *, PyObject *storage, PyObject *index, PyObject *value);
    PyObject *(*list_extend)(void *, PyObject *list, PyObject *iterable);
    PyObject *(*list_append)(void *, PyObject *list, PyObject *elem);
    PyObject *(*list_get_size)(void *, PyObject *list);
    PyObject *(*list_iter)(void *, PyObject *list);
    PyObject *(*list_iterator_next)(void *, PyObject *iterator);
    PyObject *(*symbolic_isinstance)(void *, PyObject *on, PyObject *type);
    int (*nb_add)(void *, PyObject *left, PyObject *right);
    int (*nb_subtract)(void *, PyObject *left, PyObject *right);
    int (*nb_multiply)(void *, PyObject *left, PyObject *right);
    int (*nb_remainder)(void *, PyObject *left, PyObject *right);
    int (*nb_divmod)(void *, PyObject *left, PyObject *right);
    int (*nb_bool)(void *, PyObject *on);
    int (*nb_int)(void *, PyObject *on);
    int (*nb_lshift)(void *, PyObject *left, PyObject *right);
    int (*nb_rshift)(void *, PyObject *left, PyObject *right);
    int (*nb_and)(void *, PyObject *left, PyObject *right);
    int (*nb_xor)(void *, PyObject *left, PyObject *right);
    int (*nb_or)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_add)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_subtract)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_multiply)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_remainder)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_lshift)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_rshift)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_and)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_xor)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_or)(void *, PyObject *left, PyObject *right);
    int (*nb_floor_divide)(void *, PyObject *left, PyObject *right);
    int (*nb_true_divide)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_floor_divide)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_true_divide)(void *, PyObject *left, PyObject *right);
    int (*nb_matrix_multiply)(void *, PyObject *left, PyObject *right);
    int (*nb_inplace_matrix_multiply)(void *, PyObject *left, PyObject *right);
    int (*sq_length)(void *, PyObject *);
    int (*sq_concat)(void *, PyObject *left, PyObject *right);
    int (*sq_inplace_concat)(void *, PyObject *left, PyObject *right);
    int (*mp_subscript)(void *, PyObject *storage, PyObject *index);
    int (*mp_ass_subscript)(void *, PyObject *storage, PyObject *index, PyObject *value);
    int (*tp_richcompare)(void *, int op, PyObject *left, PyObject *right);
    int (*tp_iter)(void *, PyObject *);
    int (*tp_iternext)(void *, PyObject *);
    PyObject *(*symbolic_virtual_unary_fun)(void *, PyObject *);
    PyObject *(*symbolic_virtual_binary_fun)(void *, PyObject *left, PyObject *right);
    int (*lost_symbolic_value)(void *, const char *description);
    void *virtual_tp_richcompare;
    void *virtual_tp_iter;
    void *virtual_nb_add;
    void *virtual_nb_subtract;
    void *virtual_nb_multiply;
    void *virtual_nb_matrix_multiply;
    void *virtual_mp_subscript;
    PyObject *(*approximation_builtin_len)(PyObject *);
    PyObject *(*approximation_builtin_isinstance)(PyObject *, PyObject *);
    PyObject *(*approximation_list_richcompare)(PyObject *, PyObject *, int op);
    int (*fixate_type)(void *, PyObject *);
    unary_handler default_unary_handler;
    binary_handler default_binary_handler;
    ternary_handler default_ternary_handler;
    ternary_notify default_ternary_notify;
    char msg_buffer[5000];
} SymbolicAdapter;

#include "wrapper.h"

PyAPI_FUNC(int) register_symbolic_tracing(PyObject *func, SymbolicAdapter *adapter);
PyAPI_FUNC(SymbolicAdapter*) create_new_adapter(void *param);
PyAPI_FUNC(PyObject*) SymbolicAdapter_run(PyObject *self, PyObject *function, Py_ssize_t n, PyObject *const *args);
PyObject *make_call_symbolic_handler(SymbolicAdapter *adapter, int event_type, int event_id, int nargs, PyObject *const *args);
int SymbolicAdapter_CheckExact(PyObject *obj);

#endif //CPYTHON_SYMBOLICADAPTER_H
