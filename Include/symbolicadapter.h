#ifndef CPYTHON_SYMBOLICADAPTER_H
#define CPYTHON_SYMBOLICADAPTER_H

#include "Python.h"

#define SymbolicAdapterTypeName "ibmviqhlye.___SymbolicAdapter___ibmviqhlye"

typedef PyObject *(*symbolic_handler_callable)(Py_ssize_t n, PyObject *const *, void *);

typedef struct {
    PyObject_HEAD
    symbolic_handler_callable handler;
    void *handler_param;
    PyObject *ready_wrapper_types;
    char inside_handler;
} SymbolicAdapter;

#include "wrapper.h"

PyAPI_FUNC(SymbolicAdapter*) create_new_adapter(symbolic_handler_callable handler, void *param);
SymbolicAdapter *create_new_adapter_obj(PyObject *callable);

PyAPI_FUNC(PyObject*) SymbolicAdapter_run(PyObject *self, PyObject *function, Py_ssize_t n, PyObject *const *args);

PyObject *make_call(PyObject *self, Py_ssize_t nargs, PyObject *const *args);
PyObject *make_call_with_meta(PyObject *self, Py_ssize_t nargs, PyObject *const *args, PyObject *kwargs);
PyObject *make_call_symbolic_handler(SymbolicAdapter *adapter, Py_ssize_t nargs, PyObject *const *args);

#endif //CPYTHON_SYMBOLICADAPTER_H
