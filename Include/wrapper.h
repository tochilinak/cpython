
#ifndef CPYTHON_WRAPPER_H
#define CPYTHON_WRAPPER_H

#include "Python.h"

#define WrapperTypeName "___wrapper___ibmviqhlye"
#define SYMBOLIC_HEADER "___symbolic___ibmviqhlye"
#define CONCRETE_HEADER "___concrete___ibmviqhlye"
#define WRAPPER_DICT_HEADER "___wrapper_holder___ibmviqhlye"

PyObject *wrap_concrete_object(PyObject *obj, PyObject *ready_wrapper_types);
PyObject *unwrap(PyObject *obj);
int is_wrapped(PyObject *obj);
int is_valid_object(PyObject *obj);

#endif //CPYTHON_WRAPPER_H
