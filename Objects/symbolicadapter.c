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

    if (!adapter->inside_handler) {
        frame->f_trace_opcodes = 1;
    } else {
        frame->f_trace_opcodes = 0;
    }

    PyObject *result = 0;

    if (what == PyTrace_OPCODE && !adapter->ignore) {
        PyObject *args[] = {(PyObject *) frame};
        result = make_call_symbolic_handler(adapter, SYM_EVENT_TYPE_NOTIFY, SYM_EVENT_ID_INSTRUCTION, 1, args);
    } else if (what == PyTrace_RETURN && !adapter->ignore) {
        result = make_call_symbolic_handler(adapter, SYM_EVENT_TYPE_NOTIFY, SYM_EVENT_ID_RETURN, 0, 0);
    }

    return !(result == Py_None || result == 0);
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

    PyObject *args_for_handler[] = { function };
    make_call_symbolic_handler(adapter, SYM_EVENT_TYPE_NOTIFY, SYM_EVENT_ID_PYTHON_FUNCTION_CALL, 1, args_for_handler);
    PyObject *result = Py_TYPE(function)->tp_call(function, wrappers, 0);
    // printf("result: %p %p %p\n", result, PyErr_Occurred(), PyCell_Get(cell));

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

static SymbolicAdapter *
create_new_adapter_(symbolic_handler_callable handler, PyObject *ready_wrapper_types, void *handler_param) {
    SymbolicAdapter *result = PyObject_New(SymbolicAdapter, &SymbolicAdapter_Type);
    Py_INCREF(ready_wrapper_types);
    result->handler = handler;
    result->handler_param = handler_param;
    result->ready_wrapper_types = ready_wrapper_types;
    result->inside_handler = 0;
    result->ignore = 0;
    return result;
}

SymbolicAdapter *
create_new_adapter(symbolic_handler_callable handler, void *param) {
    PyObject *ready_wrapper_types = PyDict_New();
    return create_new_adapter_(handler, ready_wrapper_types, param);
}

PyObject *
make_call_symbolic_handler(SymbolicAdapter *adapter, int event_type, int event_id, int nargs, PyObject *const *args) {
    adapter->inside_handler++;
    PyObject *result = adapter->handler(event_type, event_id, nargs, args, adapter->handler_param);
    adapter->inside_handler--;
    return result;
}

int
SymbolicAdapter_CheckExact(PyObject *obj) {
    return Py_TYPE(obj) == &SymbolicAdapter_Type;
}

void *virtual_tp_richcompare = 0;