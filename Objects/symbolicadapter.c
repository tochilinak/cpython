#include "Python.h"
#include "symbolicadapter.h"

#include "pycore_frame.h"

static void
adapter_dealloc(PyObject *op) {
    SymbolicAdapter *adapter = (SymbolicAdapter *) op;
    Py_DECREF(adapter->ready_wrapper_types);
    Py_TYPE(op)->tp_free(op);
}

static int
trace_function(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg) {
    SymbolicAdapter *adapter = (SymbolicAdapter *) obj;
    int result = 0;
    frame->f_trace_opcodes = 1;
    if (what == PyTrace_OPCODE && !adapter->ignore) {
        result = adapter->instruction(adapter->handler_param, frame);
    } else if (what == PyTrace_RETURN && !adapter->ignore) {
        // result = make_call_symbolic_handler(adapter, SYM_EVENT_TYPE_NOTIFY, SYM_EVENT_ID_RETURN, 0, 0);
    }

    return result;
}

int
register_symbolic_tracing(PyObject *func, SymbolicAdapter *adapter) {
    if (!PyFunction_Check(func)) {
        char buf[1000];
        sprintf(buf, "Wrong type of callable: %s", Py_TYPE(func)->tp_name);
        PyErr_SetString(PyExc_TypeError, buf);
        return 1;
    }
    PyObject *obj = PyFunction_GET_CODE(func);
    if (!PyCode_Check(obj)) {
        PyErr_SetString(PyExc_TypeError, "Wrong type of code object");
        return 1;
    }
    PyCodeObject *code = (PyCodeObject *) obj;
    if (!PyTuple_CheckExact(code->co_consts)) {
        PyErr_SetString(PyExc_TypeError, "Wrong type of consts holder");
        return 1;
    }

    PyTupleObject *consts = (PyTupleObject *) code->co_consts;
    Py_ssize_t n_consts = PyTuple_GET_SIZE(consts);
    PyObject *last = PyTuple_GetItem(code->co_consts, n_consts - 1);
    if (last == (PyObject *) adapter)
        return 0;

    PyTupleObject *new_consts = (PyTupleObject *) PyTuple_New(n_consts + 1);
    memcpy(new_consts->ob_item, consts->ob_item, n_consts * sizeof(PyObject *));
    for (int i = 0; i < n_consts; i++)
        Py_XINCREF(PyTuple_GET_ITEM(code->co_consts, i));
    PyTuple_SET_ITEM(new_consts, n_consts, adapter);
    Py_INCREF(adapter);

    code->co_consts = (PyObject *) new_consts;
    Py_DECREF(consts);

    return 0;
}

PyObject *
SymbolicAdapter_run(PyObject *self, PyObject *function, Py_ssize_t n, PyObject *const *args) {
    SymbolicAdapter *adapter = (SymbolicAdapter *) self;
    PyObject *wrappers = PyTuple_New(n);
    for (int i = 0; i < n; i++) {
        PyObject *cur = args[i];
        if (!PyTuple_CheckExact(cur) || PyTuple_GET_SIZE(cur) != 2) {
            PyErr_SetString(PyExc_TypeError, "all arguments must be pairs (concrete, symbolic)");
            return 0;
        }
        PyTuple_SET_ITEM(wrappers, i, wrap(PyTuple_GetItem(cur, 0), PyTuple_GetItem(cur, 1), adapter));
    }

    PyGILState_STATE gil = PyGILState_Ensure();
    PyEval_SetTrace(trace_function, self);
    int r = register_symbolic_tracing(function, adapter);
    PyGILState_Release(gil);

    if (r != 0) {
        return 0;
    }

    // PyObject *args_for_handler[] = { function };
    // make_call_symbolic_handler(adapter, SYM_EVENT_TYPE_NOTIFY, SYM_EVENT_ID_PYTHON_FUNCTION_CALL, 1, args_for_handler);
    PyObject *result = Py_TYPE(function)->tp_call(function, wrappers, 0);

    gil = PyGILState_Ensure();
    PyEval_SetTrace(0, 0);
    PyGILState_Release(gil);

    Py_DECREF(wrappers);
    return result;
}

static PyObject *
adapter_run(PyObject *self, PyObject *args) {
    if (!PyTuple_CheckExact(args) || PyTuple_GET_SIZE(args) == 0) {
        PyErr_SetString(PyExc_TypeError, "Bad args");
        return 0;
    }
    PyObject *function = PyTuple_GetItem(args, 0);
    if (Py_TYPE(function) != &PyFunction_Type) {
        PyErr_SetString(PyExc_TypeError, "First argument must be Python function");
        return 0;
    }
    Py_ssize_t n = PyTuple_GET_SIZE(args) - 1;
    PyTupleObject *args_as_tuple = (PyTupleObject *) args;
    return SymbolicAdapter_run(self, function, n, args_as_tuple->ob_item + 1);
}

static PyMethodDef adapter_methods[] = {
        {"run", adapter_run, METH_VARARGS, ""},
        {NULL, NULL}
};

PyTypeObject SymbolicAdapter_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    SymbolicAdapterTypeName,                    /* tp_name */
    sizeof(SymbolicAdapter),                    /* tp_basicsize */
    0,                                          /* tp_itemsize */
    adapter_dealloc,                            /* tp_dealloc */
    0,                                          /* tp_vectorcall_offset */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_as_async */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    0,                                          /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    adapter_methods,                            /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                          /* tp_new */
    PyObject_Free,                              /* tp_free */
};

static int default_instruction(void *arg, PyFrameObject *frame) { return 0; }
static PyObject *default_create_list(void *arg, PyObject **elems) { Py_RETURN_NONE; }
static int default_unary_notify(void *arg, PyObject *on) { return 0; }
static int default_binary_notify(void *arg, PyObject *first, PyObject *second) { return 0; }
static int default_ternary_notify(void *arg, PyObject *o1, PyObject *o2, PyObject *o3) { return 0; }
static int default_fork_result(void *arg, int result) { return 0; }
static int default_tp_richcompare(void *arg, int op, PyObject *first, PyObject *second) { return 0; }
static PyObject *default_unary(void *arg, PyObject *o) { Py_RETURN_NONE; }
static PyObject *default_binary(void *arg, PyObject *left, PyObject *right) { Py_RETURN_NONE; }
static PyObject *default_ternary(void *arg, PyObject *o1, PyObject *o2, PyObject *o3) { Py_RETURN_NONE; }
static int default_set_item(void *arg, PyObject *storage, PyObject *index, PyObject *value) { return 0; }

static SymbolicAdapter *
create_new_adapter_(PyObject *ready_wrapper_types, void *handler_param) {
    SymbolicAdapter *result = PyObject_New(SymbolicAdapter, &SymbolicAdapter_Type);
    Py_INCREF(ready_wrapper_types);
    result->ignore = 0;
    result->handler_param = handler_param;
    result->ready_wrapper_types = ready_wrapper_types;
    result->instruction = default_instruction;
    result->fork_notify = default_unary_notify;
    result->fork_result = default_fork_result;
    result->load_const = default_unary;
    result->create_list = default_create_list;
    result->gt_long = default_binary;
    result->lt_long = default_binary;
    result->eq_long = default_binary;
    result->ne_long = default_binary;
    result->le_long = default_binary;
    result->ge_long = default_binary;
    result->add_long = default_binary;
    result->sub_long = default_binary;
    result->mul_long = default_binary;
    result->div_long = default_binary;
    result->rem_long = default_binary;
    result->pow_long = default_ternary;
    result->list_get_item = default_binary;
    result->list_set_item = default_set_item;
    result->list_extend = default_binary;
    result->list_append = default_binary;
    result->list_get_size = default_unary;
    result->list_iter = default_unary;
    result->list_iterator_next = default_unary;
    result->symbolic_isinstance = default_binary;
    result->nb_add = default_binary_notify;
    result->nb_subtract = default_binary_notify;
    result->nb_multiply = default_binary_notify;
    result->nb_remainder = default_binary_notify;
    result->nb_divmod = default_binary_notify;
    result->nb_bool = default_unary_notify;
    result->nb_int = default_unary_notify;
    result->nb_lshift = default_binary_notify;
    result->nb_rshift = default_binary_notify;
    result->nb_and = default_binary_notify;
    result->nb_xor = default_binary_notify;
    result->nb_or = default_binary_notify;
    result->nb_inplace_add = default_binary_notify;
    result->nb_inplace_subtract = default_binary_notify;
    result->nb_inplace_multiply = default_binary_notify;
    result->nb_inplace_remainder = default_binary_notify;
    result->nb_inplace_lshift = default_binary_notify;
    result->nb_inplace_rshift = default_binary_notify;
    result->nb_inplace_and = default_binary_notify;
    result->nb_inplace_xor = default_binary_notify;
    result->nb_inplace_or = default_binary_notify;
    result->nb_floor_divide = default_binary_notify;
    result->nb_true_divide = default_binary_notify;
    result->nb_inplace_floor_divide = default_binary_notify;
    result->nb_inplace_true_divide = default_binary_notify;
    result->nb_matrix_multiply = default_binary_notify;
    result->nb_inplace_matrix_multiply = default_binary_notify;
    result->sq_concat = default_binary_notify;
    result->sq_inplace_concat = default_binary_notify;
    result->mp_subscript = default_binary_notify;
    result->tp_richcompare = default_tp_richcompare;
    result->tp_iter = default_unary_notify;
    result->tp_iternext = default_unary_notify;
    result->symbolic_virtual_binary_fun = default_binary;
    result->virtual_tp_richcompare = 0;
    result->virtual_nb_add = 0;
    result->virtual_mp_subscript = 0;
    result->approximation_builtin_len = 0;
    result->approximation_builtin_isinstance = 0;
    result->default_unary_handler = default_unary;
    result->default_binary_handler = default_binary;
    result->default_ternary_handler = default_ternary;
    result->default_ternary_notify = default_ternary_notify;
    return result;
}

SymbolicAdapter *
create_new_adapter(void *param) {
    PyObject *ready_wrapper_types = PyDict_New();
    return create_new_adapter_(ready_wrapper_types, param);
}

int
SymbolicAdapter_CheckExact(PyObject *obj) {
    return Py_TYPE(obj) == &SymbolicAdapter_Type;
}