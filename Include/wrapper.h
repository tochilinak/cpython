
#ifndef CPYTHON_WRAPPER_H

#include "Python.h"
#include "symbolicadapter.h"
#define CPYTHON_WRAPPER_H


PyAPI_FUNC(PyObject *) wrap(PyObject *obj, PyObject *symbolic, SymbolicAdapter *adapter);
PyAPI_FUNC(PyObject *) unwrap(PyObject *obj);
PyAPI_FUNC(int) is_wrapped(PyObject *obj);
PyAPI_FUNC(PyObject *) get_symbolic(PyObject *obj);
PyAPI_FUNC(PyObject *) get_symbolic_or_none(PyObject *obj);
PyAPI_FUNC(SymbolicAdapter *) get_adapter(PyObject *obj);

//int get_sq_item_event_id(ssizeargfunc func);

#endif //CPYTHON_WRAPPER_H
