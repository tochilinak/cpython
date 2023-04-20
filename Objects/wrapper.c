#include <stddef.h>
#include "wrapper.h"


#define SLOT(func)                  PyType_Slot Wrapper_##func = {Py_##func, func};

typedef struct {
    PyObject_HEAD
    PyObject *concrete;
    PyObject *ready_wrapper_types;
} Wrapper;

static PyObject *
get_orig(PyObject *obj) {
    if (!obj)
        return NULL;
    if (is_wrapped(obj))
        return ((Wrapper *) obj)->concrete;
    return obj;
}

static PyObject *
get_ready_wrapper_types(PyObject *obj) {
    if (!obj || !is_wrapped(obj))
        return 0;
    return ((Wrapper *) obj)->ready_wrapper_types;
}

static void
tp_dealloc(PyObject *op) {
    Wrapper *wrapper = (Wrapper*) op;
    //printf("deallocating %p; type: %p. GIL: %d. type ref: %ld\n", op, Py_TYPE(op), PyGILState_Check(), Py_REFCNT(Py_TYPE(op)));
    Py_XDECREF(wrapper->concrete);
    Py_XDECREF(wrapper->ready_wrapper_types);
    Py_TYPE(op)->tp_free(op);
}

SLOT(tp_dealloc)

static PyObject *
tp_getattro(PyObject *self, PyObject *other) {
    if (Py_TYPE(other) == &PyUnicode_Type && PyUnicode_CompareWithASCIIString(other, CONCRETE_HEADER) == 0)
        return PyUnicode_FromString(Py_TYPE(((Wrapper*)self)->concrete)->tp_name);
    //printf("calling %s ", "tp_getattro");
    //PyObject_Print(other, stdout, 0);
    //printf("\n");
    PyObject *concrete_self = get_orig(self);
    PyObject *concrete_other = get_orig(other);
    if (concrete_self->ob_type->tp_getattro == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_getattro");
        return 0;
    }
    return wrap_concrete_object(concrete_self->ob_type->tp_getattro(concrete_self, concrete_other),
                                get_ready_wrapper_types(self));
}

SLOT(tp_getattro)

// must return real string
static PyObject *
tp_repr(PyObject *self) {
    //printf("calling %s on %p\n", "tp_repr", self);
    PyObject *concrete_self = get_orig(self);
    if (concrete_self->ob_type->tp_repr == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    return concrete_self->ob_type->tp_repr(concrete_self);
}

SLOT(tp_repr)

// must return real string
static PyObject *
tp_str(PyObject *self) {
    //printf("calling %s on %p\n", "tp_str", self);
    PyObject *concrete_self = get_orig(self);
    if (concrete_self->ob_type->tp_str == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    return concrete_self->ob_type->tp_str(concrete_self);
}

SLOT(tp_str)

static PyObject *
tp_richcompare(PyObject *self, PyObject *other, int op) {
    //printf("calling %s %d on %p\n", "tp_richcompare", op, self);
    PyObject *concrete_self = get_orig(self);
    PyObject *concrete_other = get_orig(other);
    if (concrete_self->ob_type->tp_richcompare == 0) {
        PyErr_SetString(PyExc_TypeError, "no richcompare");
        return 0;
    }
    return wrap_concrete_object(concrete_self->ob_type->tp_richcompare(concrete_self, concrete_other, op), get_ready_wrapper_types(self));
}

SLOT(tp_richcompare)

static PyObject *
tp_call(PyObject *self, PyObject *o1, PyObject *o2) {
    /*if (o2) {
        printf("tp_call prep started. o2: %s\n", Py_TYPE(o2)->tp_name);
    }
    if (o1) {
        printf("tp_call prep started. o1: %ld\n", PyTuple_GET_SIZE(o1));
    }*/
    PyObject *concrete_self = get_orig(self);
    PyObject *concrete_o1 = 0;
    if (o1) {
        concrete_o1 = PyTuple_New(PyTuple_GET_SIZE(o1));
        for (int i = 0; i < PyTuple_GET_SIZE(o1); i++) {
            //printf("o1 initial type: %s\n", Py_TYPE(PyTuple_GetItem(o1, i))->tp_name);
            //printf("o1 concrete type: %s\n", Py_TYPE(get_orig(PyTuple_GetItem(o1, i)))->tp_name);
            PyObject *concrete = get_orig(PyTuple_GetItem(o1, i));
            Py_XINCREF(concrete);
            PyTuple_SetItem(concrete_o1, i, concrete);
        }
    }
    if (o2) {
        Py_ssize_t pos = 0;
        PyObject *key = 0, *value = 0;
        while (PyDict_Next(o2, &pos, &key, &value)) {
            //printf("key type: %s\n", Py_TYPE(key)->tp_name);
            if (is_wrapped(value)) {
                PyObject *concrete = get_orig(value);
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
    return wrap_concrete_object(Py_TYPE(concrete_self)->tp_call(concrete_self, concrete_o1, o2), get_ready_wrapper_types(self));
}

SLOT(tp_call)

int
tp_setattro(PyObject *self, PyObject *attr, PyObject *value)
{
    //printf("calling %s on %p\n", "tp_setattro", self);
    PyObject *concrete_self = get_orig(self);
    PyObject *concrete_attr = get_orig(attr);
    PyObject *concrete_value = get_orig(value);
    if (concrete_self->ob_type->tp_setattro == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_setattro");
        return 0;
    }

    return concrete_self->ob_type->tp_setattro(concrete_self, concrete_attr, concrete_value);
}

SLOT(tp_setattro)

PyObject *
tp_iter(PyObject *self) {
    PyObject *concrete_self = get_orig(self);

    if (concrete_self->ob_type->tp_iter == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_iter");
        return 0;
    }

    return wrap_concrete_object(concrete_self->ob_type->tp_iter(concrete_self), get_ready_wrapper_types(self));
}

SLOT(tp_iter)

PyObject *
tp_iternext(PyObject *self) {
    PyObject *concrete_self = get_orig(self);

    if (concrete_self->ob_type->tp_iternext == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_iternext");
        return 0;
    }

    return wrap_concrete_object(concrete_self->ob_type->tp_iternext(concrete_self), get_ready_wrapper_types(self));
}

SLOT(tp_iternext)

#define INQUIRY_NUMBER(func)        int \
                                    func(PyObject *self) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = get_orig(self); \
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
                                        PyObject *concrete_self = get_orig(self); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                              PyErr_SetString(PyExc_TypeError, "no as"); \
                                              return 0; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                             PyErr_SetString(PyExc_TypeError, "no func");\
                                             return 0; \
                                        } \
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as->func(concrete_self), get_ready_wrapper_types(self)); \
                                    }

#define BINARY_FUN_AS(func, tp_as)  static PyObject * \
                                    func(PyObject *self, PyObject *other) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = get_orig(self); \
                                        PyObject *concrete_other = get_orig(other); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                              /*PyErr_SetString(PyExc_TypeError, "no number");*/ \
                                              return Py_NotImplemented; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                             /*PyErr_SetString(PyExc_TypeError, "no func");*/ \
                                             return Py_NotImplemented; \
                                        } \
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as->func(concrete_self, concrete_other), get_ready_wrapper_types(self)); \
                                    }

#define TERNARY_FUN_AS(func, tp_as) static PyObject * \
                                    func(PyObject *self, PyObject *o1, PyObject *o2) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = get_orig(self); \
                                        PyObject *concrete_o1 = get_orig(o1); \
                                        PyObject *concrete_o2 = get_orig(o2); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                            /*PyErr_SetString(PyExc_TypeError, "no number");*/ \
                                            return Py_NotImplemented; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                             /*PyErr_SetString(PyExc_TypeError, "no func");*/ \
                                             return Py_NotImplemented; \
                                        } \
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as->func(concrete_self, concrete_o1, concrete_o2), get_ready_wrapper_types(self)); \
                                    }

#define LEN_FUN_AS(func, tp_as)     static Py_ssize_t \
                                    func(PyObject *self) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = get_orig(self); \
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
                                        PyObject *concrete_self = get_orig(self); \
                                        if (concrete_self->ob_type->tp_as == 0) { \
                                            PyErr_SetString(PyExc_TypeError, "no as"); \
                                            return 0; \
                                        } \
                                        if (concrete_self->ob_type->tp_as->func == 0) { \
                                             PyErr_SetString(PyExc_TypeError, "no func"); \
                                             return 0; \
                                        } \
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as->func(concrete_self, i), get_ready_wrapper_types(self)); \
                                    }

#define SIZEOBJARG_AS(func, tp_as)  static int \
                                    func(PyObject *self, Py_ssize_t i, PyObject *other) \
                                    { \
                                        /*printf("calling %s on %p\n", #func, self);*/ \
                                        PyObject *concrete_self = get_orig(self); \
                                        PyObject *concrete_other = get_orig(other); \
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
                                        PyObject *concrete_self = get_orig(self); \
                                        PyObject *concrete_o1 = get_orig(o1); \
                                        PyObject *concrete_o2 = get_orig(o2); \
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
                                        PyObject *concrete_self = get_orig(self); \
                                        PyObject *concrete_other = get_orig(other); \
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
    PyObject *concrete_self = get_orig(self);
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
    PyObject *concrete_self = get_orig(self);
    if (concrete_self->ob_type->tp_as_number == 0) {
        PyErr_SetString(PyExc_TypeError, "no as");
        return 0;
    }
    if (concrete_self->ob_type->tp_as_number->nb_float == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    return wrap_concrete_object(concrete_self->ob_type->tp_as_number->nb_float(concrete_self),
                                get_ready_wrapper_types(self));
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
    PyObject *concrete_self = get_orig(self);
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
        COUNT_OR_SET(tp_repr);
        COUNT_OR_SET(tp_call);
        COUNT_OR_SET(tp_str);
        COUNT_OR_SET(tp_getattro);
        COUNT_OR_SET(tp_setattro);
        COUNT_OR_SET(tp_richcompare);
        COUNT_OR_SET(tp_iter);
        COUNT_OR_SET(tp_iternext);
    }
    slots[i++] = Wrapper_tp_dealloc;
    slots[i++] = final_slot;
    return slots;
}

PyTypeObject *
create_new_wrapper_type(PyObject *concrete) {
    printf("Creating new wrapper type\n");
    PyType_Slot *slots = create_slot_list(concrete);
    PyType_Spec spec = {
            WrapperTypeName,
            sizeof(Wrapper),
            0,
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HEAPTYPE,
            slots
    };
    PyTypeObject *result = (PyTypeObject*) PyType_FromSpec(&spec);
    PyType_Ready(result);
    PyMem_Free(slots);
    return result;
}

int
is_wrapped(PyObject *obj) {
    if (!obj || !obj->ob_type)
        return 0;
    return strcmp(obj->ob_type->tp_name, WrapperTypeName) == 0;
}

int
is_valid_object(PyObject *obj) {
    return obj && obj->ob_type;
}

PyObject *
wrap_concrete_object(PyObject *obj, PyObject *ready_wrapper_types) {
    if (!obj)
        return 0;
    if (obj == Py_NotImplemented)
        return Py_NotImplemented;
    Py_INCREF(obj);
    Py_INCREF(ready_wrapper_types);
    PyTypeObject *wrapper_type = (PyTypeObject *) PyDict_GetItem(ready_wrapper_types, (PyObject *)Py_TYPE(obj));
    if (!wrapper_type)
        wrapper_type = create_new_wrapper_type(obj);

    PyDict_SetItem(ready_wrapper_types, (PyObject *)Py_TYPE(obj), (PyObject *)wrapper_type);

    // TODO: update type wrapper
    Wrapper *result = PyObject_New(Wrapper, wrapper_type);
    result->concrete = obj;
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