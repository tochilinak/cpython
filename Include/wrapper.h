
#ifndef CPYTHON_WRAPPER_H

#include "Python.h"
#include "symbolicadapter.h"
#define CPYTHON_WRAPPER_H

#define WrapperTypeName "ibmviqhlye.___wrapper___ibmviqhlye"
#define SYMBOLIC_HEADER "___symbolic___ibmviqhlye"
#define CONCRETE_HEADER "___concrete___ibmviqhlye"
#define SYMBOLIC_ADAPTER_HEADER "___adapter___ibmviqhlye"

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

PyObject *wrap(PyObject *obj, PyObject *symbolic, SymbolicAdapter *adapter);
PyObject *unwrap(PyObject *obj);
int is_wrapped(PyObject *obj);
PyObject *get_symbolic(PyObject *obj);

// PyObject *default_symbolic_handler(Py_ssize_t n, PyObject *const *args, PyObject *callable);

#endif //CPYTHON_WRAPPER_H
