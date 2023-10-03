#ifndef CPYTHON_SYMBOLICADAPTER_H
#define CPYTHON_SYMBOLICADAPTER_H

#include "Python.h"

#define SymbolicAdapterTypeName "ibmviqhlye.___SymbolicAdapter___ibmviqhlye"

typedef PyObject *(*unary_handler)(void *, PyObject *o);
typedef PyObject *(*binary_handler)(void *, PyObject *left, PyObject *right);
typedef PyObject *(*ternary_handler)(void *, PyObject *o1, PyObject *o2, PyObject *o3);
typedef int (*ternary_notify)(void *, PyObject *o1, PyObject *o2, PyObject *o3);
typedef void (*runnable)();

typedef struct {
    PyObject_HEAD
    void *handler_param;
    PyObject *ready_wrapper_types;
    PyObject *global_symbolic_clones_dict;
    int ignore;
    void *inside_wrapper_tp_call;
    int (*instruction)(void *, PyFrameObject *frame);
    int (*fork_notify)(void *, PyObject *on);
    int (*fork_result)(void *, PyObject *on, int result);
    int (*function_call)(void *, PyObject *code);
    int (*function_return)(void *, PyObject *code);
    int (*unpack)(void *, PyObject *iterable, int count);
    int (*is_op)(void *, PyObject *left, PyObject *right);
    int (*none_check)(void *, PyObject *on);
    PyObject *(*symbolic_tp_call)(void *, PyObject *on, PyObject *args, PyObject *kwargs);
    PyObject *(*standard_tp_getattro)(void *, PyObject *obj, PyObject *name);
    PyObject *(*load_const)(void *, PyObject *obj);
    PyObject *(*create_list)(void *, PyObject **elems);
    PyObject *(*create_tuple)(void *, PyObject **elems);
    PyObject *(*create_range)(void *, PyObject *start, PyObject *stop, PyObject *step);
    PyObject *(*create_slice)(void *, PyObject *start, PyObject *stop, PyObject *step);
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
    PyObject *(*true_div_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*rem_long)(void *, PyObject *left, PyObject *right);
    PyObject *(*pow_long)(void *, PyObject *base, PyObject *pow, PyObject *mod);
    PyObject *(*gt_float)(void *, PyObject *left, PyObject *right);
    PyObject *(*lt_float)(void *, PyObject *left, PyObject *right);
    PyObject *(*eq_float)(void *, PyObject *left, PyObject *right);
    PyObject *(*ne_float)(void *, PyObject *left, PyObject *right);
    PyObject *(*le_float)(void *, PyObject *left, PyObject *right);
    PyObject *(*ge_float)(void *, PyObject *left, PyObject *right);
    PyObject *(*add_float)(void *, PyObject *left, PyObject *right);
    PyObject *(*sub_float)(void *, PyObject *left, PyObject *right);
    PyObject *(*mul_float)(void *, PyObject *left, PyObject *right);
    PyObject *(*div_float)(void *, PyObject *left, PyObject *right);
    PyObject *(*bool_and)(void *, PyObject *, PyObject *);
    PyObject *(*list_get_item)(void *, PyObject *storage, PyObject *index);
    int (*list_set_item)(void *, PyObject *storage, PyObject *index, PyObject *value);
    PyObject *(*list_extend)(void *, PyObject *list, PyObject *iterable);
    PyObject *(*list_append)(void *, PyObject *list, PyObject *elem);
    PyObject *(*list_get_size)(void *, PyObject *list);
    PyObject *(*list_iter)(void *, PyObject *list);
    PyObject *(*list_iterator_next)(void *, PyObject *iterator);
    PyObject *(*list_concat)(void *, PyObject *, PyObject *);
    PyObject *(*list_inplace_concat)(void *, PyObject *, PyObject *);
    PyObject *(*list_pop)(void *, PyObject *);
    PyObject *(*list_pop_ind)(void *, PyObject *, PyObject *);
    int (*list_insert)(void *, PyObject *, PyObject *, PyObject *);
    PyObject *(*tuple_get_size)(void *, PyObject *tuple);
    PyObject *(*tuple_get_item)(void *, PyObject *tuple, PyObject *index);
    PyObject *(*tuple_iter)(void *, PyObject *tuple);
    PyObject *(*tuple_iterator_next)(void *, PyObject *iterator);
    PyObject *(*range_iter)(void *, PyObject *range);
    PyObject *(*range_iterator_next)(void *, PyObject *iterator);
    PyObject *(*symbolic_isinstance)(void *, PyObject *on, PyObject *type);
    PyObject *(*symbolic_int_cast)(void *, PyObject *);
    PyObject *(*symbolic_float_cast)(void *, PyObject *);
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
    int (*tp_getattro)(void *, PyObject *on, PyObject *name);
    int (*tp_iter)(void *, PyObject *);
    int (*tp_iternext)(void *, PyObject *);
    PyObject *(*symbolic_virtual_unary_fun)(void *, PyObject *);
    PyObject *(*symbolic_virtual_binary_fun)(void *, PyObject *left, PyObject *right);
    int (*lost_symbolic_value)(void *, const char *description);
    void *virtual_tp_richcompare;
    void *virtual_tp_getattro;
    void *virtual_tp_iter;
    void *virtual_nb_add;
    void *virtual_nb_subtract;
    void *virtual_nb_multiply;
    void *virtual_nb_matrix_multiply;
    void *virtual_mp_subscript;
    PyObject *(*approximation_builtin_len)(PyObject *);
    PyObject *(*approximation_builtin_isinstance)(PyObject *, PyObject *);
    PyObject *(*approximation_list_richcompare)(PyObject *, PyObject *, int op);
    PyObject *(*approximation_list_repeat)(PyObject *, PyObject *);
    PyObject *(*approximation_list_slice_get_item)(PyObject *, PyObject *);
    PyObject *(*approximation_range)(void *adapter, PyObject *args);
    PyObject *(*approximation_builtin_sum)(PyObject *);
    int (*fixate_type)(void *, PyObject *);
    unary_handler default_unary_handler;
    binary_handler default_binary_handler;
    ternary_handler default_ternary_handler;
    ternary_notify default_ternary_notify;
    char msg_buffer[5000];
} SymbolicAdapter;

#include "wrapper.h"

PyAPI_FUNC(int) register_symbolic_tracing(PyObject *func, SymbolicAdapter *adapter);
PyAPI_FUNC(SymbolicAdapter*) create_new_adapter(void *param, PyObject *global_symbolic_clones_dict);
PyAPI_FUNC(PyObject*) SymbolicAdapter_run(PyObject *self, PyObject *function, Py_ssize_t n, PyObject *const *args, runnable before_call, runnable after_call);
int SymbolicAdapter_CheckExact(PyObject *obj);

#endif //CPYTHON_SYMBOLICADAPTER_H
