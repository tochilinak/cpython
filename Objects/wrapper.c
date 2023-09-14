#include <stddef.h>
#include "wrapper.h"
#include "approximation_defs.h"

#define SLOT(func)                  PyType_Slot Wrapper_##func = {Py_##func, func};

char const * const wrapper_type_name = "ibmviqhlye.___wrapper___ibmviqhlye";

typedef struct {
    PyObject_HEAD
    PyObject *concrete;
    PyObject *symbolic;
    SymbolicAdapter *adapter;
} Wrapper;

SymbolicAdapter *
get_adapter(PyObject *obj) {
    if (!obj || !is_wrapped(obj))
        return 0;
    return ((Wrapper *) obj)->adapter;
}

PyObject *
get_symbolic(PyObject *obj) {
    if (!obj || !is_wrapped(obj))
        return 0;
    return ((Wrapper *) obj)->symbolic;
}

PyObject *
get_symbolic_or_none(PyObject *obj) {
    PyObject *res = get_symbolic(obj);
    if (!res)
        return Py_None;
    return res;
}

static void
tp_dealloc(PyObject *op) {
    //printf("DELETING: %p\n", op);
    //fflush(stdout);
    Wrapper *wrapper = (Wrapper*) op;
    Py_XDECREF(wrapper->concrete);
    Py_XDECREF(wrapper->symbolic);
    Py_XDECREF(wrapper->adapter);
    Py_TYPE(op)->tp_free(op);
}
SLOT(tp_dealloc)

static PyObject *
tp_getattr(PyObject *self, char *attr) {
    PyObject *concrete_self = unwrap(self);
    SymbolicAdapter *adapter = get_adapter(self);
    if (adapter->lost_symbolic_value(adapter->handler_param, "tp_getattr")) return 0;
    return wrap(concrete_self->ob_type->tp_getattr(concrete_self, attr), 0, adapter);
}
SLOT(tp_getattr)

static int
tp_setattr(PyObject *self, char *attr, PyObject *value) {
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_value = unwrap(value);
    return concrete_self->ob_type->tp_setattr(concrete_self, attr, concrete_value);
}
SLOT(tp_setattr)

// better not to wrap: may be a special method (like __repr__). This method is used in namedtuple
static PyObject *
tp_descr_get(PyObject *self, PyObject *obj, PyObject *type) {
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_obj = unwrap(obj);
    PyObject *concrete_type = unwrap(type);
    SymbolicAdapter *adapter = get_adapter(self);
    assert(adapter != 0);
    if (adapter->lost_symbolic_value(adapter->handler_param, "tp_descr_get")) return 0;
    return concrete_self->ob_type->tp_descr_get(concrete_self, concrete_obj, concrete_type);
}
SLOT(tp_descr_get)

static int
tp_descr_set(PyObject *self, PyObject *obj, PyObject *type) {
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_obj = unwrap(obj);
    PyObject *concrete_type = unwrap(type);
    return concrete_self->ob_type->tp_descr_set(concrete_self, concrete_obj, concrete_type);
}
SLOT(tp_descr_set)

static PyObject *
tp_getattro(PyObject *self, PyObject *other) {
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_getattro == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_getattro");
        return 0;
    }

    PyObject *concrete_other = unwrap(other);
    SymbolicAdapter *adapter = get_adapter(self);
    PyObject *symbolic_self = get_symbolic_or_none(self);
    PyObject *symbolic_other = get_symbolic_or_none(other);
    assert(adapter != 0);

    if (adapter->tp_getattro(adapter->handler_param, symbolic_self, symbolic_other))
        return 0;

    PyObject *concrete_result = concrete_self->ob_type->tp_getattro(concrete_self, concrete_other);
    if (!concrete_result)
        return 0;

    PyObject *symbolic_result = Py_None;
    if (concrete_self->ob_type->tp_getattro == PyObject_GenericGetAttr) {
        if (adapter->fixate_type(adapter->handler_param, symbolic_self)) return 0;
        symbolic_result = adapter->standard_tp_getattro(adapter->handler_param, symbolic_self, symbolic_other);
        if (!symbolic_result) return 0;
    } else if (concrete_self->ob_type->tp_getattro == adapter->virtual_tp_getattro) {
        symbolic_result = adapter->symbolic_virtual_binary_fun(adapter->handler_param, symbolic_self, symbolic_other);
        if (!symbolic_result) return 0;
    }

    if (symbolic_result == Py_None) {
        sprintf(adapter->msg_buffer, "tp_getattro on %s", Py_TYPE(self)->tp_name);
        if (adapter->lost_symbolic_value(adapter->handler_param, adapter->msg_buffer)) return 0;
    }

    return wrap(concrete_result, symbolic_result, adapter);
}
SLOT(tp_getattro)

// must return real string
static PyObject *
tp_repr(PyObject *self) {
    //printf("calling %s on %p\n", "tp_repr", self);
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_repr == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_repr");
        return 0;
    }
    return concrete_self->ob_type->tp_repr(concrete_self);
}
SLOT(tp_repr)

// must return real string
static PyObject *
tp_str(PyObject *self) {
    //printf("calling %s on %p\n", "tp_str", self);
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_str == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    return concrete_self->ob_type->tp_str(concrete_self);
}
SLOT(tp_str)

static PyObject *
approximate_tp_richcompare(SymbolicAdapter *adapter, PyObject *self, PyObject *other, int op, int *approximated) {
    *approximated = 0;
    PyObject *concrete_self = unwrap(self);
    if (Py_TYPE(concrete_self)->tp_richcompare == PyList_Type.tp_richcompare && adapter->approximation_list_richcompare) {
        *approximated = 1;
        return adapter->approximation_list_richcompare(self, other, op);
    }
    return 0;
}

static binary_handler
tp_richcompare_handler_getter(SymbolicAdapter *adapter, richcmpfunc fun, int op) {
    if (fun == PyLong_Type.tp_richcompare) {
        if (op == Py_GT) return adapter->gt_long;
        if (op == Py_LT) return adapter->lt_long;
        if (op == Py_EQ) return adapter->eq_long;
        if (op == Py_NE) return adapter->ne_long;
        if (op == Py_GE) return adapter->ge_long;
        if (op == Py_LE) return adapter->le_long;
    } else if (fun == adapter->virtual_tp_richcompare) {
        return adapter->symbolic_virtual_binary_fun;
    }
    return adapter->default_binary_handler;
}

static PyObject *
tp_richcompare(PyObject *self, PyObject *other, int op) {
    //printf("calling %s %d on %p\n", "tp_richcompare", op, self);
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_other = unwrap(other);
    if (concrete_self->ob_type->tp_richcompare == 0) {
        PyErr_SetString(PyExc_TypeError, "no richcompare");
        return 0;
    }
    SymbolicAdapter *adapter = get_adapter(self);
    int approximated;
    PyObject *r = approximate_tp_richcompare(adapter, self, other, op, &approximated);
    if (approximated)
        return r;

    PyObject *symbolic_self = get_symbolic_or_none(self);
    PyObject *symbolic_other = get_symbolic_or_none(other);
    if (adapter->tp_richcompare(adapter->handler_param, op, symbolic_self, symbolic_other) != 0)
        return 0;

    PyObject *concrete_result = concrete_self->ob_type->tp_richcompare(concrete_self, concrete_other, op);
    PyObject *symbolic = Py_None;
    if (concrete_result != Py_NotImplemented) {
        symbolic = tp_richcompare_handler_getter(adapter, concrete_self->ob_type->tp_richcompare, op)(adapter->handler_param, symbolic_self, symbolic_other);
        if (!symbolic) return 0;
    }
    if (concrete_result != Py_NotImplemented && symbolic == Py_None && symbolic_self != Py_None) {
        assert(adapter != 0);
        sprintf(adapter->msg_buffer, "tp_richcompare of %s and %s", Py_TYPE(concrete_self)->tp_name, Py_TYPE(concrete_other)->tp_name);
        if (adapter->lost_symbolic_value(adapter->handler_param, adapter->msg_buffer)) return 0;
    }
    return wrap(concrete_result, symbolic, adapter);
}
SLOT(tp_richcompare)

static PyObject *
approximate_tp_call(PyObject *original, PyObject *o1, PyObject *o2, SymbolicAdapter *adapter, int *called_approximation) {
    *called_approximation = 0;
    if (PyType_Check(original) && original == (PyObject *) &PyRange_Type && adapter->approximation_range && !o2) {
        *called_approximation = 1;
        return adapter->approximation_range(adapter, o1);
    }
    if (!PyCFunction_Check(original))
        return 0;
    void *c_method = ((PyCFunctionObject *) original)->m_ml->ml_meth;
    if (c_method == EXPORT_FOR_APPROXIMATION_BUILTIN_LEN && adapter->approximation_builtin_len) {
        if (o2 || !PyTuple_Check(o1) || PyTuple_GET_SIZE(o1) != 1)
            return 0;
        PyObject *obj = PyTuple_GetItem(o1, 0);
        *called_approximation = 1;
        return adapter->approximation_builtin_len(obj);

    } else if (c_method == EXPORT_FOR_APPROXIMATION_BUILTIN_ISINSTANCE && adapter->approximation_builtin_isinstance) {
        if (o2 || !PyTuple_Check(o1) || PyTuple_GET_SIZE(o1) != 2)
            return 0;
        PyObject *obj = PyTuple_GetItem(o1, 0);
        PyObject *type = PyTuple_GetItem(o1, 1);
        *called_approximation = 1;
        return adapter->approximation_builtin_isinstance(obj, type);

    } else if (c_method == EXPORT_FOR_APPROXIMATION_BUILTIN_SUM && adapter->approximation_builtin_sum) {
        if (o2 || !PyTuple_Check(o1) || PyTuple_GET_SIZE(o1) != 1)
            return 0;
        *called_approximation = 1;
        return adapter->approximation_builtin_sum(PyTuple_GetItem(o1, 0));

    } else if ((c_method == EXPORT_FOR_APPROXIMATION_BUILTIN_MIN
            || c_method == EXPORT_FOR_APPROXIMATION_BUILTIN_MAX
            || c_method == EXPORT_FOR_APPROXIMATION_BUILTIN_ALL) && !o2) {
        *called_approximation = 1;
        return Py_TYPE(original)->tp_call(original, o1, 0);

    }
    return 0;
}


static PyObject *
tp_call(PyObject *self, PyObject *o1, PyObject *o2) {
    PyObject *concrete_self = unwrap(self);
    PyObject *symbolic_self = get_symbolic_or_none(self);
    SymbolicAdapter *adapter = get_adapter(self);
    assert(adapter != 0);
    assert(PyTuple_Check(o1));
    assert(!o2 || PyDict_Check(o2));

    if (PyFunction_Check(concrete_self)) {
        if (register_symbolic_tracing(concrete_self, adapter))
            return 0;
        if (adapter->function_call(adapter->handler_param, PyFunction_GetCode(concrete_self)))
            return 0;
        PyObject *old = adapter->inside_wrapper_tp_call;
        adapter->inside_wrapper_tp_call = PyFunction_GetCode(concrete_self);
        PyObject *result = Py_TYPE(concrete_self)->tp_call(concrete_self, o1, o2);
        adapter->inside_wrapper_tp_call = old;
        return result;
    }

    int approximated = 0;
    PyObject *r = approximate_tp_call(concrete_self, o1, o2, adapter, &approximated);
    if (approximated)
        return r;

    PyObject *concrete_o1 = 0;
    PyObject *symbolic_o1 = 0;
    if (o1) {
        concrete_o1 = PyTuple_New(PyTuple_GET_SIZE(o1));
        symbolic_o1 = PyTuple_New(PyTuple_GET_SIZE(o1));
        for (int i = 0; i < PyTuple_GET_SIZE(o1); i++) {
            PyObject *wrapped = PyTuple_GetItem(o1, i);
            PyObject *concrete = unwrap(wrapped);
            PyObject *symbolic = get_symbolic_or_none(wrapped);
            Py_XINCREF(concrete);
            Py_XINCREF(symbolic);
            PyTuple_SetItem(concrete_o1, i, concrete);
            PyTuple_SetItem(symbolic_o1, i, symbolic);
        }
    }
    // TODO: symbolic o2
    if (o2) {
        Py_ssize_t pos = 0;
        PyObject *key = 0, *value = 0;
        while (PyDict_Next(o2, &pos, &key, &value)) {
            if (is_wrapped(value)) {
                PyObject *concrete = unwrap(value);
                Py_XINCREF(concrete);
                PyDict_SetItem(o2, key, concrete);
            }
        }
    }
    if (Py_TYPE(concrete_self)->tp_call == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_call");
        return 0;
    }
    PyObject *concrete_result = Py_TYPE(concrete_self)->tp_call(concrete_self, concrete_o1, o2);
    PyObject *symbolic_result = adapter->symbolic_tp_call(adapter->handler_param, symbolic_self, symbolic_o1, 0);
    Py_XDECREF(concrete_o1);
    Py_XDECREF(symbolic_o1);
    if (!symbolic_result) return 0;
    if (symbolic_result == Py_None) {
        if (PyCFunction_Check(concrete_self)) {
            const char *owner = "<null>";
            if (((PyCFunctionObject *) concrete_self)->m_self) {
                owner = Py_TYPE(((PyCFunctionObject *) concrete_self)->m_self)->tp_name;
            }
            sprintf(adapter->msg_buffer, "Built-in method %s of %s",
                    ((PyCFunctionObject *) concrete_self)->m_ml->ml_name, owner);
        } else if (PyType_Check(concrete_self)) {
            sprintf(adapter->msg_buffer, "Call of type %s", ((PyTypeObject *) concrete_self)->tp_name);
        } else {
            sprintf(adapter->msg_buffer, "Some callable of type %s at %p", Py_TYPE(concrete_self)->tp_name,
                    concrete_self);
        }
        if (adapter->lost_symbolic_value(adapter->handler_param, adapter->msg_buffer)) return 0;
    }
    return wrap(concrete_result, symbolic_result, adapter);
}
SLOT(tp_call)

int
tp_setattro(PyObject *self, PyObject *attr, PyObject *value)
{
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_attr = unwrap(attr);
    PyObject *concrete_value = unwrap(value);
    if (concrete_self->ob_type->tp_setattro == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_setattro");
        return 0;
    }

    return concrete_self->ob_type->tp_setattro(concrete_self, concrete_attr, concrete_value);
}
SLOT(tp_setattro)

static unary_handler
get_tp_iter_handler(SymbolicAdapter *adapter, getiterfunc func) {
    if (func == PyList_Type.tp_iter)
        return adapter->list_iter;
    if (func == adapter->virtual_tp_iter)
        return adapter->symbolic_virtual_unary_fun;
    if (func == PyTuple_Type.tp_iter)
        return adapter->tuple_iter;
    if (func == PyRange_Type.tp_iter)
        return adapter->range_iter;
    return adapter->default_unary_handler;
}

PyObject *
tp_iter(PyObject *self) {
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_iter == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_iter");
        return 0;
    }

    PyObject *symbolic_self = get_symbolic_or_none(self);
    SymbolicAdapter *adapter = get_adapter(self);
    if (adapter->tp_iter(adapter->handler_param, symbolic_self))
        return 0;
    PyObject *symbolic_result = get_tp_iter_handler(adapter, concrete_self->ob_type->tp_iter)(adapter->handler_param, symbolic_self);
    if (!symbolic_result)
        return 0;

    if (symbolic_result == Py_None && symbolic_self != Py_None) {
        assert(adapter != 0);
        sprintf(adapter->msg_buffer, "tp_iter of %s", Py_TYPE(concrete_self)->tp_name);
        if (adapter->lost_symbolic_value(adapter->handler_param, adapter->msg_buffer)) return 0;
    }

    return wrap(concrete_self->ob_type->tp_iter(concrete_self), symbolic_result, adapter);
}
SLOT(tp_iter)

static unary_handler
get_tp_iternext_handler(SymbolicAdapter *adapter, iternextfunc func) {
    if (func == PyListIter_Type.tp_iternext)
        return adapter->list_iterator_next;
    if (func == PyRangeIter_Type.tp_iternext)
        return adapter->range_iterator_next;
    if (func == PyTupleIter_Type.tp_iternext)
        return adapter->tuple_iterator_next;
    return adapter->default_unary_handler;
}

PyObject *
tp_iternext(PyObject *self) {
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_iternext == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_iternext");
        return 0;
    }
    PyObject *symbolic_self = get_symbolic_or_none(self);
    SymbolicAdapter *adapter = get_adapter(self);
    if (adapter->tp_iternext(adapter->handler_param, symbolic_self))
        return 0;
    PyObject *concrete_result = concrete_self->ob_type->tp_iternext(concrete_self);  // this might be NULL (iterator ended) but we still should call symbolic part
    PyObject *symbolic_result = get_tp_iternext_handler(adapter, concrete_self->ob_type->tp_iternext)(adapter->handler_param, symbolic_self);
    if (!symbolic_result)
        return 0;

    if (symbolic_result == Py_None && symbolic_self != Py_None && concrete_result) {
        assert(adapter != 0);
        sprintf(adapter->msg_buffer, "tp_iternext of %s", Py_TYPE(concrete_self)->tp_name);
        if (adapter->lost_symbolic_value(adapter->handler_param, adapter->msg_buffer)) return 0;
    }

    return wrap(concrete_result, symbolic_result, adapter);
}
SLOT(tp_iternext)

Py_hash_t
tp_hash(PyObject *self) {
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_hash == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_hash");
        return 0;
    }

    SymbolicAdapter *adapter = get_adapter(self);
    assert(adapter != 0);
    if (adapter->lost_symbolic_value(adapter->handler_param, "tp_hash")) return 0;

    return concrete_self->ob_type->tp_hash(concrete_self);
}
SLOT(tp_hash)

#define UNARY_FUN_AS(func, tp_as, handler_getter) \
                                    static PyObject * \
                                    func(PyObject *self) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = unwrap(self); \
                                        SymbolicAdapter *adapter = get_adapter(self); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                              PyErr_SetString(PyExc_TypeError, "no as"); \
                                              return 0; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                             PyErr_SetString(PyExc_TypeError, "no func");\
                                             return 0; \
                                        } \
                                        PyObject *concrete_result = concrete_self->ob_type->tp_as->func(concrete_self); \
                                        PyObject *symbolic = Py_None; \
                                        PyObject *symbolic_self = get_symbolic_or_none(self); \
                                        if (concrete_result != Py_NotImplemented) \
                                            symbolic = handler_getter(adapter, concrete_self->ob_type->tp_as->func)(adapter->handler_param, symbolic_self); \
                                        if (!symbolic) return 0; \
                                        if (concrete_result != Py_NotImplemented && symbolic == Py_None && symbolic_self != Py_None) { \
                                            assert(adapter != 0); \
                                            sprintf(adapter->msg_buffer, #func " on %s", Py_TYPE(concrete_self)->tp_name); \
                                            if (adapter->lost_symbolic_value(adapter->handler_param, adapter->msg_buffer)) return 0; \
                                        } \
                                        return wrap(concrete_result, symbolic, adapter); \
                                    }

#define BINARY_FUN_AS(func, tp_as, handler_getter, approximation) \
                                    static PyObject * \
                                    func(PyObject *self, PyObject *other) \
                                    { \
                                        SymbolicAdapter *adapter = get_adapter(self); \
                                        if (!adapter) \
                                            adapter = get_adapter(other); \
                                        assert(adapter != 0); \
                                        int approximated; \
                                        PyObject *r = approximation(adapter, self, other, &approximated); \
                                        if (approximated) \
                                            return r; \
                                        if (adapter->func(adapter->handler_param, get_symbolic_or_none(self), get_symbolic_or_none(other))) return 0; \
                                        PyObject *concrete_self = unwrap(self); \
                                        PyObject *concrete_other = unwrap(other); \
                                        PyObject *result = Py_NotImplemented; \
                                        binaryfunc fun = 0; \
                                        if (concrete_self->ob_type->tp_as && concrete_self->ob_type->tp_as->func) \
                                            fun = concrete_self->ob_type->tp_as->func; \
                                        binaryfunc result_fun = fun; \
                                        for (int i = 0; i <= 1; i++) { \
                                            if (fun) { \
                                                result = fun(concrete_self, concrete_other); \
                                                result_fun = fun;    \
                                            } \
                                            if (result != Py_NotImplemented) \
                                                break; \
                                            if (concrete_other->ob_type->tp_as && concrete_other->ob_type->tp_as->func) { \
                                                fun = concrete_other->ob_type->tp_as->func; \
                                            } \
                                        } \
                                        PyObject *symbolic = Py_None; \
                                        PyObject *symbolic_self = get_symbolic_or_none(self); \
                                        PyObject *symbolic_other = get_symbolic_or_none(other); \
                                        if (result != Py_NotImplemented) { \
                                            if (adapter->fixate_type(adapter->handler_param, symbolic_self)) return 0; \
                                            if (adapter->fixate_type(adapter->handler_param, symbolic_other)) return 0; \
                                            symbolic = handler_getter(adapter, result_fun)(adapter->handler_param, get_symbolic_or_none(self), get_symbolic_or_none(other)); \
                                            if (!symbolic) return 0; \
                                        } \
                                        if (result && result != Py_NotImplemented && symbolic == Py_None && symbolic_self != Py_None && symbolic_other != Py_None) { \
                                            assert(adapter != 0); \
                                            sprintf(adapter->msg_buffer, #func " on %s and %s", Py_TYPE(concrete_self)->tp_name, Py_TYPE(concrete_other)->tp_name); \
                                            if (adapter->lost_symbolic_value(adapter->handler_param, adapter->msg_buffer)) return 0; \
                                        } \
                                        return wrap(result, symbolic, adapter); \
                                    }

#define TERNARY_FUN_AS(func, tp_as, handler_getter) \
                                    static PyObject * \
                                    func(PyObject *self, PyObject *o1, PyObject *o2) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = unwrap(self); \
                                        PyObject *concrete_o1 = unwrap(o1); \
                                        PyObject *concrete_o2 = unwrap(o2); \
                                        SymbolicAdapter *adapter = get_adapter(self); \
                                        PyObject *result = Py_NotImplemented; \
                                        ternaryfunc fun = 0; \
                                        if (concrete_self->ob_type->tp_as && concrete_self->ob_type->tp_as->func) \
                                            fun = concrete_self->ob_type->tp_as->func; \
                                        ternaryfunc result_func = fun; \
                                        for (int i = 0; i <= 1; i++) { \
                                            if (fun) { \
                                                result = fun(concrete_self, concrete_o1, concrete_o2); \
                                                result_func = fun; \
                                            } \
                                            if (result != Py_NotImplemented) \
                                                break; \
                                            if (concrete_o1->ob_type->tp_as && concrete_o1->ob_type->tp_as->func) { \
                                                fun = concrete_o1->ob_type->tp_as->func; \
                                            } \
                                        } \
                                        PyObject *symbolic = Py_None; \
                                        PyObject *symbolic_self = get_symbolic_or_none(self); \
                                        PyObject *symbolic_o1 = get_symbolic_or_none(o1); \
                                        PyObject *symbolic_o2 = get_symbolic_or_none(o2); \
                                        if (result != Py_NotImplemented) { \
                                            symbolic = handler_getter(adapter, result_func)(adapter->handler_param, symbolic_self, symbolic_o1, symbolic_o2); \
                                            if (!symbolic) return 0; \
                                        } \
                                        if (result != Py_NotImplemented && symbolic == Py_None && symbolic_self != Py_None && symbolic_o1 != Py_None) { \
                                            assert(adapter != 0); \
                                            sprintf(adapter->msg_buffer, #func " on %s and %s and %s", Py_TYPE(concrete_self)->tp_name, Py_TYPE(concrete_o1)->tp_name, Py_TYPE(concrete_o2)->tp_name); \
                                            if (adapter->lost_symbolic_value(adapter->handler_param, adapter->msg_buffer)) return 0; \
                                        } \
                                        return wrap(result, symbolic, adapter); \
                                    }

#define LEN_FUN_AS(func, tp_as)     static Py_ssize_t \
                                    func(PyObject *self) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = unwrap(self); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                            PyErr_SetString(PyExc_TypeError, "no as"); \
                                            return -1; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                            PyErr_SetString(PyExc_TypeError, "no func"); \
                                            return -1; \
                                        } \
                                        SymbolicAdapter *adapter = get_adapter(self); \
                                        assert(adapter); \
                                        /* TODO */              \
                                        if (adapter->sq_length(adapter->handler_param, get_symbolic_or_none(self))) return -1; \
                                        return concrete_self->ob_type->tp_as->func(concrete_self); \
                                    }

// this one returns unwrapped value
#define SIZEARG_FUN_AS(func, tp_as) static PyObject * \
                                    func(PyObject *self, Py_ssize_t i) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = unwrap(self); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                            PyErr_SetString(PyExc_TypeError, "no as"); \
                                            return 0; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                             PyErr_SetString(PyExc_TypeError, "no func"); \
                                             return 0; \
                                        } \
                                        return concrete_self->ob_type->tp_as->func(concrete_self, i); \
                                    }

#define SIZEOBJARG_AS(func, tp_as)  static int \
                                    func(PyObject *self, Py_ssize_t i, PyObject *other) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = unwrap(self); \
                                        PyObject *concrete_other = unwrap(other); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                            PyErr_SetString(PyExc_TypeError, "no as"); \
                                            return 0; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                             PyErr_SetString(PyExc_TypeError, "no func"); \
                                             return 0; \
                                        } \
                                        return concrete_self->ob_type->tp_as->func(concrete_self, i, concrete_other); \
                                    }

#define OBJOBJ_AS(func, tp_as)      static int \
                                    func(PyObject *self, PyObject *other) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = unwrap(self); \
                                        PyObject *concrete_other = unwrap(other); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                            PyErr_SetString(PyExc_TypeError, "no as"); \
                                            return 0; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                             PyErr_SetString(PyExc_TypeError, "no func"); \
                                             return 0; \
                                        } \
                                        return concrete_self->ob_type->tp_as->func(concrete_self, concrete_other); \
                                    }

static unary_handler
default_unary_handler_getter(SymbolicAdapter *adapter, void *func) {
    return adapter->default_unary_handler;
}

static binary_handler
default_binary_handler_getter(SymbolicAdapter *adapter, void *func) {
    return adapter->default_binary_handler;
}

static PyObject *
default_binary_approximation(SymbolicAdapter *adapter, PyObject *o1, PyObject *o2, int *approximated) {
    *approximated = 0;
    return 0;
}

static ternary_handler
default_ternary_handler_getter(SymbolicAdapter *adapter, void *func) {
    return adapter->default_ternary_handler;
}

static binary_handler
get_nb_add_handler(SymbolicAdapter *adapter, binaryfunc func) {
    if (func == PyLong_Type.tp_as_number->nb_add)
        return adapter->add_long;
    if (func == adapter->virtual_nb_add)
        return adapter->symbolic_virtual_binary_fun;
    return adapter->default_binary_handler;
}
BINARY_FUN_AS(nb_add, tp_as_number, get_nb_add_handler, default_binary_approximation)
SLOT(nb_add)

static binary_handler
get_nb_sub_handler(SymbolicAdapter *adapter, binaryfunc func) {
    if (func == PyLong_Type.tp_as_number->nb_subtract)
        return adapter->sub_long;
    if (func == adapter->virtual_nb_subtract)
        return adapter->symbolic_virtual_binary_fun;
    return adapter->default_binary_handler;
}
BINARY_FUN_AS(nb_subtract, tp_as_number, get_nb_sub_handler, default_binary_approximation)
SLOT(nb_subtract)

static binary_handler
get_nb_mult_handler(SymbolicAdapter *adapter, binaryfunc func) {
    if (func == PyLong_Type.tp_as_number->nb_multiply)
        return adapter->mul_long;
    if (func == adapter->virtual_nb_multiply)
        return adapter->symbolic_virtual_binary_fun;
    return adapter->default_binary_handler;
}
BINARY_FUN_AS(nb_multiply, tp_as_number, get_nb_mult_handler, default_binary_approximation)
SLOT(nb_multiply)

static binary_handler
get_nb_rem_handler(SymbolicAdapter *adapter, binaryfunc func) {
    if (func == PyLong_Type.tp_as_number->nb_remainder)
        return adapter->rem_long;
    return adapter->default_binary_handler;
}
BINARY_FUN_AS(nb_remainder, tp_as_number, get_nb_rem_handler, default_binary_approximation)
SLOT(nb_remainder)

BINARY_FUN_AS(nb_divmod, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_divmod)

static ternary_handler
get_nb_power_handler(SymbolicAdapter *adapter, ternaryfunc func) {
    if (func == PyLong_Type.tp_as_number->nb_power)
        return adapter->pow_long;
    return adapter->default_ternary_handler;
}
TERNARY_FUN_AS(nb_power, tp_as_number, get_nb_power_handler)
SLOT(nb_power)

static unary_handler
get_nb_neg_handler(SymbolicAdapter *adapter, unaryfunc func) {
    //if (func == PyLong_Type.tp_as_number->nb_negative)
    //    return TODO;
    return adapter->default_unary_handler;
}
UNARY_FUN_AS(nb_negative, tp_as_number, get_nb_neg_handler)
SLOT(nb_negative)

UNARY_FUN_AS(nb_positive, tp_as_number, default_unary_handler_getter)
SLOT(nb_positive)

UNARY_FUN_AS(nb_absolute, tp_as_number, default_unary_handler_getter)
SLOT(nb_absolute)

int nb_bool(PyObject *self) {
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_as_number == 0) {
        PyErr_SetString(PyExc_TypeError, "no number");
        return 0;
    }
    if (concrete_self->ob_type->tp_as_number->nb_bool == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    SymbolicAdapter *adapter = get_adapter(self);
    PyObject *symbolic_self = get_symbolic_or_none(self);
    if (adapter->nb_bool(adapter->handler_param, symbolic_self))
        return -1;
    return concrete_self->ob_type->tp_as_number->nb_bool(concrete_self);
}
SLOT(nb_bool)

UNARY_FUN_AS(nb_invert, tp_as_number, default_unary_handler_getter)
SLOT(nb_invert)
BINARY_FUN_AS(nb_lshift, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_lshift)
BINARY_FUN_AS(nb_rshift, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_rshift)
BINARY_FUN_AS(nb_and, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_and)
BINARY_FUN_AS(nb_xor, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_xor)
BINARY_FUN_AS(nb_or, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_or)

// must return int
static PyObject *nb_int(PyObject *self) {
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_as_number == 0) {
        PyErr_SetString(PyExc_TypeError, "no as");
        return 0;
    }
    if (concrete_self->ob_type->tp_as_number->nb_int == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    SymbolicAdapter *adapter = get_adapter(self);
    PyObject *symbolic_self = get_symbolic_or_none(self);
    if (adapter->nb_int(adapter->handler_param, symbolic_self))
        return 0;
    return concrete_self->ob_type->tp_as_number->nb_int(concrete_self);
}
SLOT(nb_int)

// must return float
static PyObject *
nb_float(PyObject *self) {
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_as_number == 0) {
        PyErr_SetString(PyExc_TypeError, "no as_number");
        return 0;
    }
    if (concrete_self->ob_type->tp_as_number->nb_float == 0) {
        PyErr_SetString(PyExc_TypeError, "no nb_float");
        return 0;
    }
    return concrete_self->ob_type->tp_as_number->nb_float(concrete_self);
}
SLOT(nb_float)

BINARY_FUN_AS(nb_inplace_add, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_add)
BINARY_FUN_AS(nb_inplace_subtract, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_subtract)
BINARY_FUN_AS(nb_inplace_multiply, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_multiply)
BINARY_FUN_AS(nb_inplace_remainder, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_remainder)
TERNARY_FUN_AS(nb_inplace_power, tp_as_number, default_ternary_handler_getter)
SLOT(nb_inplace_power)
BINARY_FUN_AS(nb_inplace_lshift, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_lshift)
BINARY_FUN_AS(nb_inplace_rshift, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_rshift)
BINARY_FUN_AS(nb_inplace_and, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_and)
BINARY_FUN_AS(nb_inplace_xor, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_xor)
BINARY_FUN_AS(nb_inplace_or, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_or)

static binary_handler
get_nb_floor_div_handler(SymbolicAdapter *adapter, binaryfunc func) {
    if (func == PyLong_Type.tp_as_number->nb_floor_divide)
        return adapter->div_long;
    return adapter->default_binary_handler;
}
BINARY_FUN_AS(nb_floor_divide, tp_as_number, get_nb_floor_div_handler, default_binary_approximation)
SLOT(nb_floor_divide)

BINARY_FUN_AS(nb_true_divide, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_true_divide)
BINARY_FUN_AS(nb_inplace_floor_divide, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_floor_divide)
BINARY_FUN_AS(nb_inplace_true_divide, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_true_divide)

// must return integer
static PyObject *
nb_index(PyObject *self) {
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_as_number == 0) {
        PyErr_SetString(PyExc_TypeError, "no as_number");
        return 0;
    }
    if (concrete_self->ob_type->tp_as_number->nb_index == 0) {
        PyErr_SetString(PyExc_TypeError, "no nb_index");
        return 0;
    }
    return concrete_self->ob_type->tp_as_number->nb_index(concrete_self);
}
SLOT(nb_index)

static binary_handler
get_nb_matrix_multiply_handler(SymbolicAdapter *adapter, binaryfunc func) {
    if (func == adapter->virtual_nb_matrix_multiply)
        return adapter->symbolic_virtual_binary_fun;
    return adapter->default_binary_handler;
}
BINARY_FUN_AS(nb_matrix_multiply, tp_as_number, get_nb_matrix_multiply_handler, default_binary_approximation)
SLOT(nb_matrix_multiply)
BINARY_FUN_AS(nb_inplace_matrix_multiply, tp_as_number, default_binary_handler_getter, default_binary_approximation)
SLOT(nb_inplace_matrix_multiply)

LEN_FUN_AS(sq_length, tp_as_sequence)
SLOT(sq_length)

static binary_handler
get_sq_concat_handler(SymbolicAdapter *adapter, binaryfunc func) {
    if (func == PyList_Type.tp_as_sequence->sq_concat)
        return adapter->list_concat;
    return adapter->default_binary_handler;
}

BINARY_FUN_AS(sq_concat, tp_as_sequence, get_sq_concat_handler, default_binary_approximation)
SLOT(sq_concat)

SIZEARG_FUN_AS(sq_repeat, tp_as_sequence)
SLOT(sq_repeat)

/*
int get_sq_item_event_id(ssizeargfunc func) {
    if (func == PyList_Type.tp_as_sequence->sq_item)
        return SYM_EVENT_ID_LIST_GET_ITEM;
    return -1;
}
*/
SIZEARG_FUN_AS(sq_item, tp_as_sequence)
SLOT(sq_item)

SIZEOBJARG_AS(sq_ass_item, tp_as_sequence)
SLOT(sq_ass_item)
OBJOBJ_AS(sq_contains, tp_as_sequence)
SLOT(sq_contains)

static binary_handler
get_sq_inplace_concat_handler(SymbolicAdapter *adapter, binaryfunc func) {
    if (func == PyList_Type.tp_as_sequence->sq_inplace_concat) {
        return adapter->list_inplace_concat;
    }
    return adapter->default_binary_handler;
}
BINARY_FUN_AS(sq_inplace_concat, tp_as_sequence, get_sq_inplace_concat_handler, default_binary_approximation)
SLOT(sq_inplace_concat)

SIZEARG_FUN_AS(sq_inplace_repeat, tp_as_sequence)
SLOT(sq_inplace_repeat)

LEN_FUN_AS(mp_length, tp_as_mapping)
SLOT(mp_length)

static binary_handler
get_mp_subscript_handler(SymbolicAdapter *adapter, binaryfunc func) {
    if (func == PyList_Type.tp_as_mapping->mp_subscript)
        return adapter->list_get_item;
    if (func == adapter->virtual_mp_subscript)
        return adapter->symbolic_virtual_binary_fun;
    return adapter->default_binary_handler;
}
static PyObject *
mp_subscript_approximation(SymbolicAdapter *adapter, PyObject *self, PyObject *other, int *approximated) {
    if (Py_TYPE(unwrap(self)) == &PyList_Type && Py_TYPE(unwrap(other)) == &PySlice_Type && adapter->approximation_list_slice_get_item) {
        *approximated = 1;
        return adapter->approximation_list_slice_get_item(self, other);
    }
    *approximated = 0;
    return 0;
}
BINARY_FUN_AS(mp_subscript, tp_as_mapping, get_mp_subscript_handler, mp_subscript_approximation)
SLOT(mp_subscript)

static ternary_notify
get_mp_ass_subscript_handler(SymbolicAdapter *adapter, objobjargproc func) {
    if (func == PyList_Type.tp_as_mapping->mp_ass_subscript)
        return adapter->list_set_item;
    return adapter->default_ternary_notify;
}

static int
mp_ass_subscript(PyObject *self, PyObject *index, PyObject *value) {
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_o1 = unwrap(index);
    PyObject *concrete_o2 = unwrap(value);
    PyObject *symbolic_self = get_symbolic_or_none(self);
    PyObject *symbolic_index = get_symbolic_or_none(index);
    PyObject *symbolic_value = get_symbolic_or_none(value);
    if (concrete_self->ob_type->tp_as_mapping == 0) {
        PyErr_SetString(PyExc_TypeError, "no mp_ass_subscript");
        return -1;
    }
    if (concrete_self->ob_type->tp_as_mapping->mp_ass_subscript == 0) {
        PyErr_SetString(PyExc_TypeError, "no mp_ass_subscript");
        return -1;
    }
    SymbolicAdapter *adapter = get_adapter(self);
    if (adapter->mp_ass_subscript(adapter->handler_param, symbolic_self, symbolic_index, symbolic_value))
        return -1;
    objobjargproc func = concrete_self->ob_type->tp_as_mapping->mp_ass_subscript;
    int concrete_result = func(concrete_self, concrete_o1, concrete_o2);
    if (adapter) {
        int res = get_mp_ass_subscript_handler(adapter, func)(
                adapter->handler_param, symbolic_self, symbolic_index, symbolic_value);
        if (res != 0) return -1;
    }
    return concrete_result;
}

SLOT(mp_ass_subscript)

PyType_Slot final_slot = {0, NULL};

#define COUNT_OR_SET_FROM_METHODS(func) \
do { \
    if (concrete_methods->func) {       \
        if (count) {                    \
            counter++;                  \
        } else {                        \
            slots[i++] = Wrapper_##func;\
        }                               \
    } \
} while (0)

#define COUNT_OR_SET(func)              \
do { \
    if (Py_TYPE(concrete)->func) {      \
        if (count) {                    \
            counter++;                  \
        } else {                        \
            slots[i++] = Wrapper_##func;\
        }                               \
    } \
} while (0)

PyType_Slot *
create_slot_list(PyObject *concrete) {
    int counter = 2;
    int i = 0;
    PyType_Slot *slots = 0;
    for (int count = 1; count >= 0; count--) {
        if (!count) {
            slots = PyMem_RawMalloc(sizeof(PyType_Slot) * counter);
        }
        if (Py_TYPE(concrete)->tp_as_number) {
            PyNumberMethods *concrete_methods = Py_TYPE(concrete)->tp_as_number;
            COUNT_OR_SET_FROM_METHODS(nb_add);
            COUNT_OR_SET_FROM_METHODS(nb_subtract);
            COUNT_OR_SET_FROM_METHODS(nb_multiply);
            COUNT_OR_SET_FROM_METHODS(nb_remainder);
            COUNT_OR_SET_FROM_METHODS(nb_divmod);
            COUNT_OR_SET_FROM_METHODS(nb_power);
            COUNT_OR_SET_FROM_METHODS(nb_negative);
            COUNT_OR_SET_FROM_METHODS(nb_positive);
            COUNT_OR_SET_FROM_METHODS(nb_absolute);
            COUNT_OR_SET_FROM_METHODS(nb_bool);
            COUNT_OR_SET_FROM_METHODS(nb_invert);
            COUNT_OR_SET_FROM_METHODS(nb_lshift);
            COUNT_OR_SET_FROM_METHODS(nb_rshift);
            COUNT_OR_SET_FROM_METHODS(nb_and);
            COUNT_OR_SET_FROM_METHODS(nb_xor);
            COUNT_OR_SET_FROM_METHODS(nb_or);
            COUNT_OR_SET_FROM_METHODS(nb_int);
            COUNT_OR_SET_FROM_METHODS(nb_float);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_add);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_subtract);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_multiply);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_remainder);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_power);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_lshift);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_rshift);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_and);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_xor);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_or);
            COUNT_OR_SET_FROM_METHODS(nb_floor_divide);
            COUNT_OR_SET_FROM_METHODS(nb_true_divide);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_floor_divide);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_true_divide);
            COUNT_OR_SET_FROM_METHODS(nb_index);
            COUNT_OR_SET_FROM_METHODS(nb_matrix_multiply);
            COUNT_OR_SET_FROM_METHODS(nb_inplace_matrix_multiply);
        }
        if (Py_TYPE(concrete)->tp_as_sequence) {
            PySequenceMethods *concrete_methods = Py_TYPE(concrete)->tp_as_sequence;
            COUNT_OR_SET_FROM_METHODS(sq_length);
            COUNT_OR_SET_FROM_METHODS(sq_concat);
            COUNT_OR_SET_FROM_METHODS(sq_repeat);
            COUNT_OR_SET_FROM_METHODS(sq_item);
            COUNT_OR_SET_FROM_METHODS(sq_ass_item);
            COUNT_OR_SET_FROM_METHODS(sq_contains);
            COUNT_OR_SET_FROM_METHODS(sq_inplace_concat);
            COUNT_OR_SET_FROM_METHODS(sq_inplace_repeat);
        }
        if (Py_TYPE(concrete)->tp_as_mapping) {
            PyMappingMethods *concrete_methods = Py_TYPE(concrete)->tp_as_mapping;
            COUNT_OR_SET_FROM_METHODS(mp_length);
            COUNT_OR_SET_FROM_METHODS(mp_subscript);
            COUNT_OR_SET_FROM_METHODS(mp_ass_subscript);
        }
        COUNT_OR_SET(tp_getattr);
        COUNT_OR_SET(tp_setattr);
        // as_async, as_buffer TODO?
        COUNT_OR_SET(tp_repr);
        COUNT_OR_SET(tp_hash);
        COUNT_OR_SET(tp_call);
        COUNT_OR_SET(tp_str);
        COUNT_OR_SET(tp_getattro);
        COUNT_OR_SET(tp_setattro);
        COUNT_OR_SET(tp_richcompare);
        COUNT_OR_SET(tp_iter);
        COUNT_OR_SET(tp_iternext);
        COUNT_OR_SET(tp_descr_get);
        COUNT_OR_SET(tp_descr_set);
    }
    slots[i++] = Wrapper_tp_dealloc;
    slots[i++] = final_slot;
    return slots;
}

static PyTypeObject *
create_new_wrapper_type(PyObject *concrete) {
    PyType_Slot *slots = create_slot_list(concrete);
    PyType_Spec spec = {
            wrapper_type_name,
            sizeof(Wrapper),
            0,
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
            slots
    };
    PyTypeObject *result = (PyTypeObject*) PyType_FromSpec(&spec);
    PyMem_RawFree(slots);
    return result;
}

int
is_wrapped(PyObject *obj) {
    if (!obj || !obj->ob_type)
        return 0;
    return strcmp(Py_TYPE(obj)->tp_name, wrapper_type_name) == 0;  // TODO: do it through base type
}

PyObject *
wrap(PyObject *obj, PyObject *symbolic, SymbolicAdapter *adapter) {
    if (!obj)
        return 0;
    if (!symbolic)
        symbolic = Py_None;
    if (obj == Py_NotImplemented)
        Py_RETURN_NOTIMPLEMENTED;
    if (is_wrapped(obj)) {
        return obj;
    }
    Py_INCREF(obj);
    Py_INCREF(symbolic);
    Py_INCREF(adapter);
    PyTypeObject *wrapper_type = (PyTypeObject *) PyDict_GetItem(adapter->ready_wrapper_types, (PyObject *)Py_TYPE(obj));
    if (!wrapper_type)
        wrapper_type = create_new_wrapper_type(obj);
    if (!wrapper_type) {
        char err_str[200];
        sprintf(err_str, "Something went wrong in wrapper type creation of object of type %s", Py_TYPE(obj)->tp_name);
        PyErr_SetString(PyExc_AssertionError, err_str);
        return 0;
    }

    PyDict_SetItem(adapter->ready_wrapper_types, (PyObject *)Py_TYPE(obj), (PyObject *)wrapper_type);

    // TODO: update type wrapper
    Wrapper *result = PyObject_New(Wrapper, wrapper_type);
    result->concrete = obj;
    result->symbolic = symbolic;
    result->adapter = adapter;
    //printf("CREATED %p for %p\n", result, symbolic);
    //fflush(stdout);
    return (PyObject *) result;
}

PyObject *
unwrap(PyObject *obj) {
    if (!obj)
        return NULL;

    if (!is_wrapped(obj))
        return obj;

    Wrapper *obj_as_wrapper = (Wrapper *) obj;
    PyObject *result = obj_as_wrapper->concrete;
    return result;
}