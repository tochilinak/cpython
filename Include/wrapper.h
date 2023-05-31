
#ifndef CPYTHON_WRAPPER_H

#include "Python.h"
#include "symbolicadapter.h"
#define CPYTHON_WRAPPER_H


PyObject *wrap(PyObject *obj, PyObject *symbolic, SymbolicAdapter *adapter);
PyObject *unwrap(PyObject *obj);
int is_wrapped(PyObject *obj);
PyObject *get_symbolic(PyObject *obj);
PyObject *get_symbolic_or_none(PyObject *obj);

// PyObject *default_symbolic_handler(Py_ssize_t n, PyObject *const *args, PyObject *callable);

#endif //CPYTHON_WRAPPER_H
