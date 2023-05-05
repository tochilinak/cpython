#include <stddef.h>
#include "wrapper.h"


#define SLOT(func)                  PyType_Slot Wrapper_##func = {Py_##func, func};

typedef struct {
    PyObject_HEAD
    PyObject *concrete;
    PyObject *symbolic;
    PyObject *ready_wrapper_types;
    PyObject *symbolic_handler;
} Wrapper;

static PyObject *
get_ready_wrapper_types(PyObject *obj) {
    if (!obj || !is_wrapped(obj))
        return 0;
    return ((Wrapper *) obj)->ready_wrapper_types;
}

static PyObject *
get_symbolic_handler(PyObject *obj) {
    if (!obj || !is_wrapped(obj))
        return 0;
    return ((Wrapper *) obj)->symbolic_handler;
}

PyObject *
get_symbolic(PyObject *obj) {
    if (!obj || !is_wrapped(obj))
        return 0;
    return ((Wrapper *) obj)->symbolic;
}

static PyObject *
get_symbolic_or_none(PyObject *obj) {
    PyObject *res = get_symbolic(obj);
    if (!res)
        return Py_None;
    return res;
}

static void
tp_dealloc(PyObject *op) {
    Wrapper *wrapper = (Wrapper*) op;
    //printf("deallocating %p; type: %p. GIL: %d. type ref: %ld\n", op, Py_TYPE(op), PyGILState_Check(), Py_REFCNT(Py_TYPE(op)));
    Py_XDECREF(wrapper->concrete);
    Py_XDECREF(wrapper->symbolic);
    Py_XDECREF(wrapper->ready_wrapper_types);
    Py_XDECREF(wrapper->symbolic_handler);
    Py_TYPE(op)->tp_free(op);
}
SLOT(tp_dealloc)

PyObject *
make_call_with_meta(PyObject *self, int nargs, PyObject **args, PyObject *kwargs) {
    PyObject *tuple = PyTuple_New(nargs);
    for (int i = 0; i < nargs; i++) {
        PyObject *cur = args[i];
        if (!cur) cur = Py_None;
        Py_INCREF(cur);
        PyTuple_SetItem(tuple, i, cur);
    }
    PyObject *result = Py_TYPE(self)->tp_call(self, tuple, kwargs);
    Py_DECREF(tuple);
    return result;
}

PyObject *
make_call(PyObject *self, int nargs, PyObject **args) {
    return make_call_with_meta(self, nargs, args, 0);
}

static PyObject *
tp_getattr(PyObject *self, char *attr) {
    PyObject *concrete_self = unwrap(self);
    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_getattr_name);
    PyObject *args[] = {func_name, get_symbolic_or_none(self), Py_None};
    PyObject *symb_res = make_call(handler, 3, args);
    Py_DECREF(func_name);
    if (0 /* entering Python method */) {
        return concrete_self->ob_type->tp_getattr(self, attr);
    } else {
        return wrap(concrete_self->ob_type->tp_getattr(concrete_self, attr),symb_res,
                    get_ready_wrapper_types(self), handler);
    }
}
SLOT(tp_getattr)

static int
tp_setattr(PyObject *self, char *attr, PyObject *value) {
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_value = unwrap(value);
    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_setattr_name);
    PyObject *args[] = {func_name, get_symbolic_or_none(self), Py_None, get_symbolic_or_none(value)};
    make_call(handler, 4, args);
    Py_DECREF(func_name);
    if (0 /* entering Python method */) {
        return concrete_self->ob_type->tp_setattr(self, attr, value);
    } else {
        return concrete_self->ob_type->tp_setattr(concrete_self, attr, concrete_value);
    }
}
SLOT(tp_setattr)

// better not to wrap: may be special method (like __repr__). This method is used in namedtuple
static PyObject *
tp_descr_get(PyObject *self, PyObject *obj, PyObject *type) {
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_obj = unwrap(obj);
    PyObject *concrete_type = unwrap(type);
    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_descr_get_name);
    PyObject *args[] = {func_name, get_symbolic_or_none(self), get_symbolic_or_none(obj), get_symbolic_or_none(type)};
    make_call(handler, 4, args);
    Py_DECREF(func_name);
    if (0 /* entering Python method */) {
        return concrete_self->ob_type->tp_descr_get(self, obj, type);
    } else {
        return concrete_self->ob_type->tp_descr_get(concrete_self, concrete_obj, concrete_type);
    }
}
SLOT(tp_descr_get)

static int
tp_descr_set(PyObject *self, PyObject *obj, PyObject *type) {
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_obj = unwrap(obj);
    PyObject *concrete_type = unwrap(type);
    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_descr_set_name);
    PyObject *args[] = {func_name, get_symbolic_or_none(self), get_symbolic_or_none(obj), get_symbolic_or_none(type)};
    make_call(handler, 4, args);
    Py_DECREF(func_name);
    if (0 /* entering Python method */) {
        return concrete_self->ob_type->tp_descr_set(self, obj, type);
    } else {
        return concrete_self->ob_type->tp_descr_set(concrete_self, concrete_obj, concrete_type);
    }
}
SLOT(tp_descr_set)

static PyObject *
tp_getattro(PyObject *self, PyObject *other) {
    if (Py_TYPE(other) == &PyUnicode_Type && PyUnicode_CompareWithASCIIString(other, CONCRETE_HEADER) == 0)
        return PyUnicode_FromString(Py_TYPE(((Wrapper*)self)->concrete)->tp_name);
    //printf("calling %s ", "tp_getattro");
    //PyObject_Print(other, stdout, 0);
    //printf("\n");
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_getattro == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_getattro");
        return 0;
    }

    PyObject *concrete_other = unwrap(other);
    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_getattro_name);
    PyObject *args[] = {func_name, get_symbolic_or_none(self), get_symbolic_or_none(other)};
    PyObject *symb_result = make_call(handler, 3, args);
    Py_DECREF(func_name);
    if (0 /* entering Python method */) {
        return concrete_self->ob_type->tp_getattro(self, other);
    } else {
        return wrap(concrete_self->ob_type->tp_getattro(concrete_self, concrete_other), symb_result,
                    get_ready_wrapper_types(self), handler);
    }
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
    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_repr_name);
    PyObject *args[] = {func_name, get_symbolic_or_none(self)};
    make_call(handler, 2, args);
    Py_DECREF(func_name);
    if (0 /* entering Python method */) {
        // TODO
    } else {
        return concrete_self->ob_type->tp_repr(concrete_self);
    }
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
    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_str_name);
    PyObject *args[] = {func_name, get_symbolic_or_none(self)};
    make_call(handler, 2, args);
    Py_DECREF(func_name);
    if (0 /* entering Python method */) {
        // TODO
    } else {
        return concrete_self->ob_type->tp_str(concrete_self);
    }
}
SLOT(tp_str)

static PyObject *
tp_richcompare(PyObject *self, PyObject *other, int op) {
    //printf("calling %s %d on %p\n", "tp_richcompare", op, self);
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_other = unwrap(other);
    if (concrete_self->ob_type->tp_richcompare == 0) {
        PyErr_SetString(PyExc_TypeError, "no richcompare");
        return 0;
    }
    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_richcompare_name);
    PyObject *args[] = {func_name, PyLong_FromLong(op), get_symbolic_or_none(self), get_symbolic_or_none(other)};
    PyObject *symb_result = make_call(handler, 4, args);
    Py_DECREF(func_name);
    if (0 /* entering Python method */) {
        // TODO
    } else {
        return wrap(concrete_self->ob_type->tp_richcompare(concrete_self, concrete_other, op), symb_result,
                    get_ready_wrapper_types(self), get_symbolic_handler(self));
    }
}
SLOT(tp_richcompare)

static PyObject *
tp_call(PyObject *self, PyObject *o1, PyObject *o2) {
    PyObject *concrete_self = unwrap(self);
    PyObject *concrete_o1 = 0;
    if (o1) {
        concrete_o1 = PyTuple_New(PyTuple_GET_SIZE(o1));
        for (int i = 0; i < PyTuple_GET_SIZE(o1); i++) {
            PyObject *concrete = unwrap(PyTuple_GetItem(o1, i));
            Py_XINCREF(concrete);
            PyTuple_SetItem(concrete_o1, i, concrete);
        }
    }
    //printf("calling tp_call %s %ld\n", Py_TYPE(concrete_self)->tp_name, PyTuple_GET_SIZE(o1));
    //fflush(stdout);
    if (o2) {
        Py_ssize_t pos = 0;
        PyObject *key = 0, *value = 0;
        while (PyDict_Next(o2, &pos, &key, &value)) {
            //printf("key type: %s\n", Py_TYPE(key)->tp_name);
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
    //printf("tp_call prep ended\n");
    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_call_name);
    PyObject *args[] = {func_name, get_symbolic_or_none(self), get_symbolic_or_none(o1), get_symbolic_or_none(o2)};
    PyObject *symb_result = make_call(handler, 4, args);
    Py_DECREF(func_name);
    if (0 /* entering Python method */) {
        // TODO
    } else {
        return wrap(Py_TYPE(concrete_self)->tp_call(concrete_self, concrete_o1, o2), symb_result,
                    get_ready_wrapper_types(self), get_symbolic_handler(self));
    }
}
SLOT(tp_call)

//TODO
int
tp_setattro(PyObject *self, PyObject *attr, PyObject *value)
{
    //printf("calling %s on %p\n", "tp_setattro", self);
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

PyObject *
tp_iter(PyObject *self) {
    PyObject *concrete_self = unwrap(self);

    if (concrete_self->ob_type->tp_iter == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_iter");
        return 0;
    }

    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_iter_name);
    PyObject *args[] = {func_name, get_symbolic_or_none(self)};
    PyObject *symb_result = make_call(handler, 2, args);
    Py_DECREF(func_name);
    if (0 /* entering Python method */) {
        // TODO
    } else {
        return wrap(concrete_self->ob_type->tp_iter(concrete_self), symb_result,
                    get_ready_wrapper_types(self), get_symbolic_handler(self));
    }
}
SLOT(tp_iter)

PyObject *
tp_iternext(PyObject *self) {
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_iternext == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_iternext");
        return 0;
    }
    PyObject *handler = get_symbolic_handler(self);
    PyObject *func_name = PyUnicode_FromString(tp_iternext_name);
    PyObject *args[] = {func_name, get_symbolic_or_none(self)};
    PyObject *symb_result = make_call(handler, 2, args);
    Py_DECREF(func_name);
    return wrap(concrete_self->ob_type->tp_iternext(concrete_self), symb_result,
                get_ready_wrapper_types(self), get_symbolic_handler(self));
}
SLOT(tp_iternext)

Py_hash_t
tp_hash(PyObject *self) {
    PyObject *concrete_self = unwrap(self);
    if (concrete_self->ob_type->tp_hash == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_hash");
        return 0;
    }
    return concrete_self->ob_type->tp_hash(concrete_self);
}
SLOT(tp_hash)

#define INQUIRY_NUMBER(func)        int \
                                    func(PyObject *self) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = unwrap(self); \
                                        if (concrete_self->ob_type->tp_as_number == 0) { \
                                            PyErr_SetString(PyExc_TypeError, "no number"); \
                                            return 0; \
                                        } \
                                        if (concrete_self->ob_type->tp_as_number->func == 0) { \
                                            PyErr_SetString(PyExc_TypeError, "no func"); \
                                            return 0; \
                                        } \
                                        return concrete_self->ob_type->tp_as_number->func(concrete_self); \
                                    }

#define UNARY_FUN_AS(func, tp_as)   static PyObject * \
                                    func(PyObject *self) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = unwrap(self); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                              PyErr_SetString(PyExc_TypeError, "no as"); \
                                              return 0; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                             PyErr_SetString(PyExc_TypeError, "no func");\
                                             return 0; \
                                        } \
                                        return wrap(concrete_self->ob_type->tp_as->func(concrete_self), Py_None, \
                                                    get_ready_wrapper_types(self), get_symbolic_handler(self)); \
                                    }

#define BINARY_FUN_AS(func, tp_as)  static PyObject * \
                                    func(PyObject *self, PyObject *other) \
                                    { \
                                        PyObject *concrete_self = unwrap(self); \
                                        PyObject *concrete_other = unwrap(other); \
                                        PyObject *result = Py_NotImplemented; \
                                        binaryfunc fun = 0; \
                                        if (concrete_self->ob_type->tp_as && concrete_self->ob_type->tp_as->func) \
                                            fun = concrete_self->ob_type->tp_as->func; \
                                        for (int i = 0; i <= 1; i++) { \
                                            if (fun) { \
                                                result = fun(concrete_self, concrete_other); \
                                            } \
                                            if (result != Py_NotImplemented) \
                                                break; \
                                            if (concrete_other->ob_type->tp_as && concrete_other->ob_type->tp_as->func) { \
                                                fun = concrete_other->ob_type->tp_as->func; \
                                            } \
                                        } \
                                        return wrap(result, Py_None, get_ready_wrapper_types(self), get_symbolic_handler(self)); \
                                    }

#define TERNARY_FUN_AS(func, tp_as) static PyObject * \
                                    func(PyObject *self, PyObject *o1, PyObject *o2) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = unwrap(self); \
                                        PyObject *concrete_o1 = unwrap(o1); \
                                        PyObject *concrete_o2 = unwrap(o2); \
                                        PyObject *result = Py_NotImplemented; \
                                        ternaryfunc fun = 0; \
                                        if (concrete_self->ob_type->tp_as && concrete_self->ob_type->tp_as->func) \
                                            fun = concrete_self->ob_type->tp_as->func; \
                                        for (int i = 0; i <= 1; i++) { \
                                            if (fun) { \
                                                result = fun(concrete_self, concrete_o1, concrete_o2); \
                                            } \
                                            if (result != Py_NotImplemented) \
                                                break; \
                                            if (concrete_o1->ob_type->tp_as && concrete_o1->ob_type->tp_as->func) { \
                                                fun = concrete_o1->ob_type->tp_as->func; \
                                            } \
                                        } \
                                        return wrap(result, Py_None, \
                                                   get_ready_wrapper_types(self), get_symbolic_handler(self)); \
                                    }

#define LEN_FUN_AS(func, tp_as)     static Py_ssize_t \
                                    func(PyObject *self) \
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
                                        return concrete_self->ob_type->tp_as->func(concrete_self); \
                                    }

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
                                        return wrap(concrete_self->ob_type->tp_as->func(concrete_self, i), Py_None, \
                                                    get_ready_wrapper_types(self), get_symbolic_handler(self)); \
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

#define OBJOBJARG_AS(func, tp_as)   static int \
                                    func(PyObject *self, PyObject *o1, PyObject *o2) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = unwrap(self); \
                                        PyObject *concrete_o1 = unwrap(o1); \
                                        PyObject *concrete_o2 = unwrap(o2); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                            PyErr_SetString(PyExc_TypeError, "no as"); \
                                            return 0; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                             PyErr_SetString(PyExc_TypeError, "no func"); \
                                             return 0; \
                                        } \
                                        return concrete_self->ob_type->tp_as->func(concrete_self, concrete_o1, concrete_o2); \
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

BINARY_FUN_AS(nb_add, tp_as_number)
SLOT(nb_add)
BINARY_FUN_AS(nb_subtract, tp_as_number)
SLOT(nb_subtract)
BINARY_FUN_AS(nb_multiply, tp_as_number)
SLOT(nb_multiply)
BINARY_FUN_AS(nb_remainder, tp_as_number)
SLOT(nb_remainder)
BINARY_FUN_AS(nb_divmod, tp_as_number)
SLOT(nb_divmod)
TERNARY_FUN_AS(nb_power, tp_as_number)
SLOT(nb_power)

UNARY_FUN_AS(nb_negative, tp_as_number)
SLOT(nb_negative)

UNARY_FUN_AS(nb_positive, tp_as_number)
SLOT(nb_positive)

UNARY_FUN_AS(nb_absolute, tp_as_number)
SLOT(nb_absolute)
INQUIRY_NUMBER(nb_bool)
SLOT(nb_bool)

UNARY_FUN_AS(nb_invert, tp_as_number)
SLOT(nb_invert)
BINARY_FUN_AS(nb_lshift, tp_as_number)
SLOT(nb_lshift)
BINARY_FUN_AS(nb_rshift, tp_as_number)
SLOT(nb_rshift)
BINARY_FUN_AS(nb_and, tp_as_number)
SLOT(nb_and)
BINARY_FUN_AS(nb_xor, tp_as_number)
SLOT(nb_xor)
BINARY_FUN_AS(nb_or, tp_as_number)
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

BINARY_FUN_AS(nb_inplace_add, tp_as_number)
SLOT(nb_inplace_add)
BINARY_FUN_AS(nb_inplace_subtract, tp_as_number)
SLOT(nb_inplace_subtract)
BINARY_FUN_AS(nb_inplace_multiply, tp_as_number)
SLOT(nb_inplace_multiply)
BINARY_FUN_AS(nb_inplace_remainder, tp_as_number)
SLOT(nb_inplace_remainder)
TERNARY_FUN_AS(nb_inplace_power, tp_as_number)
SLOT(nb_inplace_power)
BINARY_FUN_AS(nb_inplace_lshift, tp_as_number)
SLOT(nb_inplace_lshift)
BINARY_FUN_AS(nb_inplace_rshift, tp_as_number)
SLOT(nb_inplace_rshift)
BINARY_FUN_AS(nb_inplace_and, tp_as_number)
SLOT(nb_inplace_and)
BINARY_FUN_AS(nb_inplace_xor, tp_as_number)
SLOT(nb_inplace_xor)
BINARY_FUN_AS(nb_inplace_or, tp_as_number)
SLOT(nb_inplace_or)
BINARY_FUN_AS(nb_floor_divide, tp_as_number)
SLOT(nb_floor_divide)
BINARY_FUN_AS(nb_true_divide, tp_as_number)
SLOT(nb_true_divide)
BINARY_FUN_AS(nb_inplace_floor_divide, tp_as_number)
SLOT(nb_inplace_floor_divide)
BINARY_FUN_AS(nb_inplace_true_divide, tp_as_number)
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

BINARY_FUN_AS(nb_matrix_multiply, tp_as_number)
SLOT(nb_matrix_multiply)
BINARY_FUN_AS(nb_inplace_matrix_multiply, tp_as_number)
SLOT(nb_inplace_matrix_multiply)

LEN_FUN_AS(sq_length, tp_as_sequence)
SLOT(sq_length)
BINARY_FUN_AS(sq_concat, tp_as_sequence)
SLOT(sq_concat)

SIZEARG_FUN_AS(sq_repeat, tp_as_sequence)
SLOT(sq_repeat)

SIZEARG_FUN_AS(sq_item, tp_as_sequence)
SLOT(sq_item)
SIZEOBJARG_AS(sq_ass_item, tp_as_sequence)
SLOT(sq_ass_item)
OBJOBJ_AS(sq_contains, tp_as_sequence)
SLOT(sq_contains)
BINARY_FUN_AS(sq_inplace_concat, tp_as_sequence)
SLOT(sq_inplace_concat)

SIZEARG_FUN_AS(sq_inplace_repeat, tp_as_sequence)
SLOT(sq_inplace_repeat)

LEN_FUN_AS(mp_length, tp_as_mapping)
SLOT(mp_length)
BINARY_FUN_AS(mp_subscript, tp_as_mapping)
SLOT(mp_subscript)
OBJOBJARG_AS(mp_ass_subscript, tp_as_mapping)
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

PyTypeObject *
create_new_wrapper_type(PyObject *concrete) {
    PyType_Slot *slots = create_slot_list(concrete);
    PyType_Spec spec = {
            WrapperTypeName,
            sizeof(Wrapper),
            0,
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
            slots
    };
    PyTypeObject *result = (PyTypeObject*) PyType_FromSpec(&spec);
    PyMem_Free(slots);
    return result;
}

int
is_wrapped(PyObject *obj) {
    if (!obj || !obj->ob_type)
        return 0;
    return strcmp(obj->ob_type->tp_name, WrapperTypeName) == 0;
}

PyObject *
wrap(PyObject *obj, PyObject *symbolic, PyObject *ready_wrapper_types, PyObject *symbolic_handler) {
    if (!obj)
        return 0;
    if (!symbolic)
        symbolic = Py_None;
    if (obj == Py_NotImplemented)
        Py_RETURN_NOTIMPLEMENTED;
    if (is_wrapped(obj)) {
        //Py_INCREF(obj);
        return obj;
    }
    Py_INCREF(obj);
    Py_INCREF(symbolic);
    Py_INCREF(ready_wrapper_types);
    Py_INCREF(symbolic_handler);
    PyTypeObject *wrapper_type = (PyTypeObject *) PyDict_GetItem(ready_wrapper_types, (PyObject *)Py_TYPE(obj));
    if (!wrapper_type)
        wrapper_type = create_new_wrapper_type(obj);
    if (!wrapper_type) {
        char err_str[200];
        sprintf(err_str, "Something went wrong in wrapper type creation of object of type %s", Py_TYPE(obj)->tp_name);
        PyErr_SetString(PyExc_AssertionError, err_str);
        return 0;
    }

    PyDict_SetItem(ready_wrapper_types, (PyObject *)Py_TYPE(obj), (PyObject *)wrapper_type);

    // TODO: update type wrapper
    Wrapper *result = PyObject_New(Wrapper, wrapper_type);
    result->concrete = obj;
    result->symbolic = symbolic;
    result->symbolic_handler = symbolic_handler;
    result->ready_wrapper_types = ready_wrapper_types;
    //printf("created %p; type: %p\n", result, Py_TYPE(result));
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