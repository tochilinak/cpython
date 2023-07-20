#ifndef CPYTHON_SYMBOLICADAPTER_H
#define CPYTHON_SYMBOLICADAPTER_H

#include "Python.h"
#include "SYMBOLIC_API.h"

#define SymbolicAdapterTypeName "ibmviqhlye.___SymbolicAdapter___ibmviqhlye"

typedef struct {
    PyObject_HEAD
    symbolic_handler_callable handler;
    void *handler_param;
    PyObject *ready_wrapper_types;
    char inside_handler;
    int ignore;
} SymbolicAdapter;

#include "wrapper.h"

int register_symbolic_tracing(PyObject *func, SymbolicAdapter *adapter);
PyAPI_FUNC(SymbolicAdapter*) create_new_adapter(symbolic_handler_callable handler, void *param);
PyAPI_FUNC(PyObject*) SymbolicAdapter_run(PyObject *self, PyObject *function, Py_ssize_t n, PyObject *const *args);
PyObject *make_call_symbolic_handler(SymbolicAdapter *adapter, int event_type, int event_id, int nargs, PyObject *const *args);
int SymbolicAdapter_CheckExact(PyObject *obj);

#endif //CPYTHON_SYMBOLICADAPTER_H
