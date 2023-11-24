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

    /** Notifies that a new byte code instruction is about to be executed. */
    int (*instruction)(void *, PyFrameObject *frame);

    /** Notifies that execution is about to fork on symbolic object `on`. */
    int (*fork_notify)(void *, PyObject *on);

    /** Notifies that the result of the fork on `on` is `result`. */
    int (*fork_result)(void *, PyObject *on, int result);

    /** Notifies that `code` is about to be executed. */
    int (*function_call)(void *, PyObject *code);

    /** Notifies that execution is about to return from `code`. */
    int (*function_return)(void *, PyObject *code);

    /** Notifies that `iterable` is about to be unpacked into `count` elements on the interpreter stack. */
    int (*unpack)(void *, PyObject *iterable, int count);

    /** Asks for result of operation `left is right` */
    int (*is_op)(void *, PyObject *left, PyObject *right);

    /** TODO */
    int (*none_check)(void *, PyObject *on);

    /** TODO */
    PyObject *(*symbolic_tp_call)(void *, PyObject *on, PyObject *args, PyObject *kwargs);

    /** TODO */
    PyObject *(*extract_self_from_method)(void *, PyObject *on);

    /** TODO */
    PyObject *(*approximate_type_call)(void *, int *approximated, PyObject *type, PyObject *args, PyObject *kwargs);

    /** TODO */
    int (*is_pycfunction_with_approximation)(void *, PyObject *symbolic_pycfunction);

    /** TODO */
    PyObject *(*extract_symbolic_self_from_pycfunction)(void *, PyObject *symbolic_pycfunction);

    /** TODO */
    PyObject *(*approximate_pycfunction_call)(void *, int *approximated, PyObject *symbolic_pycfunction, PyObject *wrapped_self, PyObject *args, PyObject *kwargs);

    /** TODO */
    PyObject *(*standard_tp_getattro)(void *, PyObject *obj, PyObject *name);

    /** TODO */
    int (*standard_tp_setattro)(void *, PyObject *obj, PyObject *name, PyObject *value);

    /** Asks for symbolic representation of constant `obj`. */
    PyObject *(*load_const)(void *, PyObject *obj);

    /** Asks for symbolic representation of list. */
    PyObject *(*create_list)(void *, PyObject **elems);

    /** Like `create_list` but for `tuple`. */
    PyObject *(*create_tuple)(void *, PyObject **elems);

    /** Asks for symbolic representation of `range` object. */
    PyObject *(*create_range)(void *, PyObject *start, PyObject *stop, PyObject *step);

    /** TODO */
    PyObject *(*create_slice)(void *, PyObject *start, PyObject *stop, PyObject *step);

    /** TODO */
    PyObject *(*create_dict)(void *, PyObject **keys, PyObject **elems);

    /** TODO */
    PyObject *(*create_dict_const_key)(void *, PyObject *keys, PyObject **elems);

    /** `>` operation on symbolic integers `left` and `right`. */
    PyObject *(*gt_long)(void *, PyObject *left, PyObject *right);

    /** `<` operation on symbolic integers `left` and `right`. */
    PyObject *(*lt_long)(void *, PyObject *left, PyObject *right);

    /** `==` operation on symbolic integers `left` and `right`. */
    PyObject *(*eq_long)(void *, PyObject *left, PyObject *right);

    /** `!=` operation on symbolic integers `left` and `right`. */
    PyObject *(*ne_long)(void *, PyObject *left, PyObject *right);

    /** `<=` operation on symbolic integers `left` and `right`. */
    PyObject *(*le_long)(void *, PyObject *left, PyObject *right);

    /** `>=` operation on symbolic integers `left` and `right`. */
    PyObject *(*ge_long)(void *, PyObject *left, PyObject *right);

    /** `+` operation on symbolic integers `left` and `right`. */
    PyObject *(*add_long)(void *, PyObject *left, PyObject *right);

    /** `-` operation on symbolic integers `left` and `right`. */
    PyObject *(*sub_long)(void *, PyObject *left, PyObject *right);

    /** `*` operation on symbolic integers `left` and `right`. */
    PyObject *(*mul_long)(void *, PyObject *left, PyObject *right);

    /** `//` operation on symbolic integers `left` and `right`. */
    PyObject *(*div_long)(void *, PyObject *left, PyObject *right);

    /** `/` operation on symbolic integers `left` and `right`. */
    PyObject *(*true_div_long)(void *, PyObject *left, PyObject *right);

    /** `%` operation on symbolic integers `left` and `right`. */
    PyObject *(*rem_long)(void *, PyObject *left, PyObject *right);

    /** TODO */
    PyObject *(*pow_long)(void *, PyObject *base, PyObject *pow, PyObject *mod);

    /** unary `-` operation on symbolic integer `on`. */
    PyObject *(*neg_long)(void *, PyObject *on);

    /** unary `+` operation on symbolic integer `on`. */
    PyObject *(*pos_long)(void *, PyObject *on);

    /** TODO */
    PyObject *(*gt_float)(void *, PyObject *left, PyObject *right);

    /** TODO */
    PyObject *(*lt_float)(void *, PyObject *left, PyObject *right);

    /** TODO */
    PyObject *(*eq_float)(void *, PyObject *left, PyObject *right);

    /** TODO */
    PyObject *(*ne_float)(void *, PyObject *left, PyObject *right);

    /** TODO */
    PyObject *(*le_float)(void *, PyObject *left, PyObject *right);

    /** TODO */
    PyObject *(*ge_float)(void *, PyObject *left, PyObject *right);

    /** TODO */
    PyObject *(*add_float)(void *, PyObject *left, PyObject *right);

    /** TODO */
    PyObject *(*sub_float)(void *, PyObject *left, PyObject *right);

    /** TODO */
    PyObject *(*mul_float)(void *, PyObject *left, PyObject *right);

    /** TODO */
    PyObject *(*div_float)(void *, PyObject *left, PyObject *right);

    /** unary `-` operation on symbolic float `on`. */
    PyObject *(*neg_float)(void *, PyObject *on);

    /** unary `+` operation on symbolic float `on`. */
    PyObject *(*pos_float)(void *, PyObject *on);

    /** Operation `storage[index]` (when concrete implementation is `PyList_Type.tp_as_mapping->mp_subscript`). */
    PyObject *(*list_get_item)(void *, PyObject *storage, PyObject *index);

    /** Notifies about `PyList_Type.tp_as_mapping->mp_ass_subscript`. All arguments are symbolic representations. */
    int (*list_set_item)(void *, PyObject *storage, PyObject *index, PyObject *value);

    /**
     * Operation https://docs.python.org/3.11/library/dis.html#opcode-LIST_EXTEND.
     * Expects resulting list as a return value.
     */
    PyObject *(*list_extend)(void *, PyObject *list, PyObject *iterable);

    /**
     * Operation Operation https://docs.python.org/3.11/library/dis.html#opcode-LIST_APPEND.
     * Expects resulting list as a return value.
     */
    PyObject *(*list_append)(void *, PyObject *list, PyObject *elem);

    /** Asks for symbolic length of symbolic list. */
    PyObject *(*list_get_size)(void *, PyObject *list);

    /** Operation `iter()` on a symbolic list. */
    PyObject *(*list_iter)(void *, PyObject *list);

    /** Operation `next()` on a symbolic list iterator (list iterator is a result of operation `iter()` on list). */
    PyObject *(*list_iterator_next)(void *, PyObject *iterator);

    /** `+` operation on symbolic lists. */
    PyObject *(*list_concat)(void *, PyObject *, PyObject *);

    /** Inplace version of `list_concat`. */
    PyObject *(*list_inplace_concat)(void *, PyObject *, PyObject *);

    /** Asks for symbolic length of symbolic tuple. */
    PyObject *(*tuple_get_size)(void *, PyObject *tuple);

    /** TODO */
    PyObject *(*tuple_get_item)(void *, PyObject *tuple, PyObject *index);

    /** Operation `iter()` on a symbolic tuple. */
    PyObject *(*tuple_iter)(void *, PyObject *tuple);

    /** Operation `next()` on a symbolic tuple iterator (tuple iterator is a result of operation `iter()` on tuple). */
    PyObject *(*tuple_iterator_next)(void *, PyObject *iterator);

    /** TODO */
    PyObject *(*dict_get_item)(void *, PyObject *self, PyObject *key);

    /** TODO */
    int (*dict_set_item)(void *, PyObject *self, PyObject *key, PyObject *value);

    /** Operation `iter()` on a symbolic representation of range object. */
    PyObject *(*range_iter)(void *, PyObject *range);

    /** Operation `next()` on a symbolic range iterator (range iterator is a result of operation `iter()` on range object). */
    PyObject *(*range_iterator_next)(void *, PyObject *iterator);

    /** Asks for a symbolic result of operation `isinstance`. */
    PyObject *(*symbolic_isinstance)(void *, PyObject *on, PyObject *type);

    /** TODO */
    int (*nb_negative)(void *, PyObject *on);

    /** TODO */
    int (*nb_positive)(void *, PyObject *on);

    /** TODO */
    int (*nb_absolute)(void *, PyObject *on);

    /** TODO */
    int (*nb_invert)(void *, PyObject *on);

    /** Notifies that `nb_add` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_add)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_subtract` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_subtract)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_multiply` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_multiply)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_remainder` is about to be performed on symbolic objects `left` and `right` */
    int (*nb_remainder)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_divmod` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_divmod)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_bool` is about to be performed on symbolic object `on`. */
    int (*nb_bool)(void *, PyObject *on);

    /** Notifies that `nb_int` is about to be performed on symbolic object `on`. */
    int (*nb_int)(void *, PyObject *on);

    /** Notifies that `nb_lshift` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_lshift)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_rshift` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_rshift)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_and` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_and)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_xor` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_xor)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_or` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_or)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_add` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_add)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_subtract` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_subtract)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_multiply` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_multiply)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_remainder` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_remainder)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_lshift` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_lshift)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_rshift` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_rshift)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_and` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_and)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_xor` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_xor)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_or` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_or)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_floor_divide` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_floor_divide)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_true_divide` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_true_divide)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_floor_divide` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_floor_divide)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_true_divide` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_true_divide)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_matrix_multiply` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_matrix_multiply)(void *, PyObject *left, PyObject *right);

    /** Notifies that `nb_inplace_matrix_multiply` is about to be performed on symbolic objects `left` and `right`. */
    int (*nb_inplace_matrix_multiply)(void *, PyObject *left, PyObject *right);

    /** Notifies that `sq_length` is about to be performed on symbolic object `on`. */
    int (*sq_length)(void *, PyObject *);

    /** Notifies that `sq_concat` is about to be performed on symbolic objects `left` and `right`. */
    int (*sq_concat)(void *, PyObject *left, PyObject *right);

    /** Notifies that `sq_inplace_concat` is about to be performed on symbolic objects `left` and `right`. */
    int (*sq_inplace_concat)(void *, PyObject *left, PyObject *right);

    /** Notifies that `mp_subscript` is about to be performed on symbolic objects `storage` and `index`. */
    int (*mp_subscript)(void *, PyObject *storage, PyObject *index);

    /** Notifies that `mp_ass_subscript` is about to be performed on symbolic objects `storage`, `index` and `value`. */
    int (*mp_ass_subscript)(void *, PyObject *storage, PyObject *index, PyObject *value);

    /** Notifies that `tp_richcompare` with operation `op` is about to be performed on symbolic objects `left` and `right`. */
    int (*tp_richcompare)(void *, int op, PyObject *left, PyObject *right);

    /** Notifies that `tp_getattro` is about to be performed on symbolic objects `on` and `name` (`name` is a symbolic representation of string). */
    int (*tp_getattro)(void *, PyObject *on, PyObject *name);

    /** TODO */
    int (*tp_setattro)(void *, PyObject *on, PyObject *name, PyObject *value);

    /** Notifies that `tp_iter` is about to be performed on symbolic object `on`. */
    int (*tp_iter)(void *, PyObject *);

    /** Notifies that `tp_iternext` is about to be performed on symbolic object `on`. */
    int (*tp_iternext)(void *, PyObject *);

    /** TODO */
    int (*tp_call)(void *, PyObject *on);

    /** TODO */
    int (*tp_hash)(void *, PyObject *on);

    /** TODO */
    int (*fixate_type)(void *, PyObject *);

    /** TODO */
    PyObject *(*symbolic_virtual_unary_fun)(void *, PyObject *);

    /** TODO */
    PyObject *(*symbolic_virtual_binary_fun)(void *, PyObject *left, PyObject *right);

    /** TODO */
    int (*lost_symbolic_value)(void *, const char *description);

    /** TODO */
    void *virtual_tp_richcompare;

    /** TODO */
    void *virtual_tp_getattro;

    /** TODO */
    void *virtual_tp_iter;

    /** TODO */
    void *virtual_tp_call;

    /** TODO */
    void *virtual_nb_add;

    /** TODO */
    void *virtual_nb_subtract;

    /** TODO */
    void *virtual_nb_multiply;

    /** TODO */
    void *virtual_nb_negative;

    /** TODO */
    void *virtual_nb_positive;

    /** TODO */
    void *virtual_nb_matrix_multiply;

    /** TODO */
    void *virtual_mp_subscript;

    /** TODO */
    PyObject *(*approximation_builtin_len)(PyObject *);

    /** TODO */
    PyObject *(*approximation_builtin_isinstance)(PyObject *, PyObject *);

    /** TODO */
    PyObject *(*approximation_list_richcompare)(PyObject *, PyObject *, int op);

    /** TODO */
    PyObject *(*approximation_list_repeat)(PyObject *, PyObject *);

    /** TODO */
    PyObject *(*approximation_list_slice_get_item)(PyObject *, PyObject *);

    /** TODO */
    PyObject *(*approximation_range)(void *adapter, PyObject *args);

    /** TODO */
    PyObject *(*approximation_builtin_sum)(PyObject *);

    /** TODO */
    int (*approximation_contains_op)(PyObject *container, PyObject *item, int *approximated);

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
