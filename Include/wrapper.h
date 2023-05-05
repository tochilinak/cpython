
#ifndef CPYTHON_WRAPPER_H
#define CPYTHON_WRAPPER_H

#include "Python.h"

#define WrapperTypeName "ibmviqhlye.___wrapper___ibmviqhlye"
#define SYMBOLIC_HEADER "___symbolic___ibmviqhlye"
#define CONCRETE_HEADER "___concrete___ibmviqhlye"
#define WRAPPER_DICT_HEADER "___wrapper_holder___ibmviqhlye"
#define SYMBOLIC_HANDLER_HEADER "___handler___ibmviqhlye"
#define INSIDE_SYMBOLIC_HANDLER_HEADER "___inside_handler___ibmviqhlye"

#define tp_getattr_name "GETATTR"
#define tp_setattr_name "SETATTR"
#define tp_descr_get_name "DESCR_GET"
#define tp_descr_set_name "DESCR_SET"
#define tp_getattro_name "GETATTRO"
#define tp_setattro_name "SETATTRO"
#define tp_repr_name "REPR"
#define tp_str_name "STR"
#define tp_richcompare_name "RICHCMP"  // followed by operation id
#define tp_call_name "CALL"
#define tp_iter_name "ITER"
#define tp_iternext_name "ITERNEXT"

PyObject *wrap(PyObject *obj, PyObject *symbolic, PyObject *ready_wrapper_types, PyObject *symbolic_handler);
PyObject *unwrap(PyObject *obj);
int is_wrapped(PyObject *obj);
PyObject *get_symbolic(PyObject *obj);

PyObject *make_call(PyObject *self, int nargs, PyObject **args);
PyObject *make_call_with_meta(PyObject *self, int nargs, PyObject **args, PyObject *kwargs);

#endif //CPYTHON_WRAPPER_H
