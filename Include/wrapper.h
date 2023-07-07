
#ifndef CPYTHON_WRAPPER_H

#include "Python.h"
#include "symbolicadapter.h"
#define CPYTHON_WRAPPER_H


PyObject *wrap(PyObject *obj, PyObject *symbolic, SymbolicAdapter *adapter);
PyObject *unwrap(PyObject *obj);
int is_wrapped(PyObject *obj);
PyObject *get_symbolic(PyObject *obj);
PyObject *get_symbolic_or_none(PyObject *obj);
SymbolicAdapter *get_adapter(PyObject *obj);

//int get_sq_item_event_id(ssizeargfunc func);

#endif //CPYTHON_WRAPPER_H
