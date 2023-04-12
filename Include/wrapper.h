
#ifndef CPYTHON_WRAPPER_H
#define CPYTHON_WRAPPER_H

#include "Python.h"

#define WrapperTypeName "___wrapper___ibmviqhlye"
#define SYMBOLIC_HEADER "___symbolic___ibmviqhlye"
#define CONCRETE_HEADER "___concrete___ibmviqhlye"

struct wrapper {
    PyObject_HEAD
    PyObject *concrete;
};

typedef struct wrapper Wrapper;

PyObject *wrap_concrete_object(PyObject *obj);
PyObject *unwrap(PyObject *obj);
int is_wrapped(PyObject *obj);

#endif //CPYTHON_WRAPPER_H
