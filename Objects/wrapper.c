#include <stddef.h>
#include "wrapper.h"

static PyObject *
get_orig(PyObject *obj) {
    if (!obj)
        return NULL;
    if (strcmp(obj->ob_type->tp_name, WrapperTypeName) == 0)
        return ((Wrapper *) obj)->concrete;
    return obj;
}

#define UNARY_FUN(func)             static PyObject * \
                                    func(PyObject *self) \
                                    { \
                                        printf("calling %s\n", #func); \
                                        PyObject *concrete_self = get_orig(self); \
                                        if (concrete_self->ob_type->func == 0) \
                                             PyErr_SetString(PyExc_TypeError, "no func"); \
                                        return wrap_concrete_object(concrete_self->ob_type->func(concrete_self)); \
                                    }

#define BINARY_FUN(func)            static PyObject * \
                                    func(PyObject *self, PyObject *other) \
                                    { \
                                        printf("calling %s\n", #func); \
                                        PyObject *concrete_self = get_orig(self); \
                                        PyObject *concrete_other = get_orig(other); \
                                        if (concrete_self->ob_type->func == 0) \
                                             PyErr_SetString(PyExc_TypeError, "no func"); \
                                        return wrap_concrete_object(concrete_self->ob_type->func(concrete_self, concrete_other)); \
                                    }

#define TERNARY_FUN(func)           static PyObject * \
                                    func(PyObject *self, PyObject *o1, PyObject *o2) \
                                    { \
                                        printf("calling %s\n", #func); \
                                        PyObject *concrete_self = get_orig(self); \
                                        PyObject *concrete_o1 = get_orig(o1); \
                                        PyObject *concrete_o2 = get_orig(o2); \
                                        if (concrete_self->ob_type->func == 0) \
                                             PyErr_SetString(PyExc_TypeError, "no func"); \
                                        return wrap_concrete_object(concrete_self->ob_type->func(concrete_self, concrete_o1, concrete_o2)); \
                                    }

static void
wrapper_dealloc(Wrapper *op) {
    Py_XDECREF(op->concrete);
    Py_TYPE(op)->tp_free((PyObject *) op);
}

static PyObject *
tp_getattro(PyObject *self, PyObject *other) {
    if (Py_TYPE(other) == &PyUnicode_Type && PyUnicode_CompareWithASCIIString(other, CONCRETE_HEADER) == 0)
        return ((Wrapper*)self)->concrete;

    printf("calling %s\n", "tp_getattro");
    PyObject *concrete_self = get_orig(self);
    PyObject *concrete_other = get_orig(other);
    if (concrete_self->ob_type->tp_getattro == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    return wrap_concrete_object(concrete_self->ob_type->tp_getattro(concrete_self, concrete_other));
}

// must return real string
static PyObject *
tp_repr(PyObject *self) {
    printf("calling %s on %p\n", "tp_repr", self);
    PyObject *concrete_self = get_orig(self);
    if (concrete_self->ob_type->tp_repr == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    return concrete_self->ob_type->tp_repr(concrete_self);
}

// must return real string
static PyObject *
tp_str(PyObject *self) {
    printf("calling %s on %p\n", "tp_str", self);
    PyObject *concrete_self = get_orig(self);
    if (concrete_self->ob_type->tp_str == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    return concrete_self->ob_type->tp_str(concrete_self);
}

static PyObject *
tp_richcompare(PyObject *self, PyObject *other, int op) {
    printf("calling %s %d on %p\n", "tp_richcompare", op, self);
    PyObject *concrete_self = get_orig(self);
    PyObject *concrete_other = get_orig(other);
    if (concrete_self->ob_type->tp_richcompare == 0) {
        PyErr_SetString(PyExc_TypeError, "no richcompare");
        return 0;
    }
    return wrap_concrete_object(concrete_self->ob_type->tp_richcompare(concrete_self, concrete_other, op));
}

TERNARY_FUN(tp_call)

int
tp_setattro(PyObject *self, PyObject *attr, PyObject *value)
{
    printf("calling %s on %p\n", "tp_setattro", self);
    PyObject *concrete_self = get_orig(self);
    PyObject *concrete_attr = get_orig(attr);
    PyObject *concrete_value = get_orig(value);
    if (concrete_self->ob_type->tp_setattro == 0) {
        PyErr_SetString(PyExc_TypeError, "no tp_setattro");
        return 0;
    }

    return concrete_self->ob_type->tp_setattro(concrete_self, concrete_attr, concrete_value);
}

#define INQUIRY_NUMBER(func)        int \
                                    func(PyObject *self) \
                                    { \
                                        printf("calling %s on %p\n", #func, self); \
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

#define UNARY_FUN_NUMBER(func)      static PyObject * \
                                    func(PyObject *self) \
                                    { \
                                        printf("calling %s on %p\n", #func, self); \
                                        PyObject *concrete_self = get_orig(self); \
                                        if (concrete_self->ob_type->tp_as_number == 0) { \
                                              PyErr_SetString(PyExc_TypeError, "no number"); \
                                              return 0; \
                                        } \
                                        if (concrete_self->ob_type->tp_as_number->func == 0) { \
                                             PyErr_SetString(PyExc_TypeError, "no func");\
                                             return 0; \
                                        } \
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as_number->func(concrete_self)); \
                                    }

#define BINARY_FUN_NUMBER(func)     static PyObject * \
                                    func(PyObject *self, PyObject *other) \
                                    { \
                                        printf("calling %s on %p\n", #func, self); \
                                        PyObject *concrete_self = get_orig(self); \
                                        PyObject *concrete_other = get_orig(other); \
                                        if (concrete_self->ob_type->tp_as_number == 0) { \
                                              /*PyErr_SetString(PyExc_TypeError, "no number");*/ \
                                              return Py_NotImplemented; \
                                        } \
                                        if (concrete_self->ob_type->tp_as_number->func == 0) { \
                                             /*PyErr_SetString(PyExc_TypeError, "no func");*/ \
                                             return Py_NotImplemented; \
                                        } \
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as_number->func(concrete_self, concrete_other)); \
                                    }

#define TERNARY_FUN_NUMBER(func)    static PyObject * \
                                    func(PyObject *self, PyObject *o1, PyObject *o2) \
                                    { \
                                        printf("calling %s on %p\n", #func, self); \
                                        PyObject *concrete_self = get_orig(self); \
                                        PyObject *concrete_o1 = get_orig(o1); \
                                        PyObject *concrete_o2 = get_orig(o2); \
                                        if (concrete_self->ob_type->tp_as_number == 0) { \
                                            /*PyErr_SetString(PyExc_TypeError, "no number");*/ \
                                            return Py_NotImplemented; \
                                        } \
                                        if (concrete_self->ob_type->tp_as_number->func == 0) { \
                                             /*PyErr_SetString(PyExc_TypeError, "no func");*/ \
                                             return Py_NotImplemented; \
                                        } \
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as_number->func(concrete_self, concrete_o1, concrete_o2)); \
                                    }

BINARY_FUN_NUMBER(nb_add)
BINARY_FUN_NUMBER(nb_subtract)
BINARY_FUN_NUMBER(nb_multiply)
BINARY_FUN_NUMBER(nb_remainder)
BINARY_FUN_NUMBER(nb_divmod)
TERNARY_FUN_NUMBER(nb_power)
UNARY_FUN_NUMBER(nb_negative)
UNARY_FUN_NUMBER(nb_positive)
UNARY_FUN_NUMBER(nb_absolute)
INQUIRY_NUMBER(nb_bool)
UNARY_FUN_NUMBER(nb_invert)
BINARY_FUN_NUMBER(nb_lshift)
BINARY_FUN_NUMBER(nb_rshift)
BINARY_FUN_NUMBER(nb_and)
BINARY_FUN_NUMBER(nb_xor)
BINARY_FUN_NUMBER(nb_or)
UNARY_FUN_NUMBER(nb_int)
UNARY_FUN_NUMBER(nb_float)
BINARY_FUN_NUMBER(nb_inplace_add)
BINARY_FUN_NUMBER(nb_inplace_subtract)
BINARY_FUN_NUMBER(nb_inplace_multiply)
BINARY_FUN_NUMBER(nb_inplace_remainder)
TERNARY_FUN_NUMBER(nb_inplace_power)
BINARY_FUN_NUMBER(nb_inplace_lshift)
BINARY_FUN_NUMBER(nb_inplace_rshift)
BINARY_FUN_NUMBER(nb_inplace_and)
BINARY_FUN_NUMBER(nb_inplace_xor)
BINARY_FUN_NUMBER(nb_inplace_or)
BINARY_FUN_NUMBER(nb_floor_divide)
BINARY_FUN_NUMBER(nb_true_divide)
BINARY_FUN_NUMBER(nb_inplace_floor_divide)
BINARY_FUN_NUMBER(nb_inplace_true_divide)
UNARY_FUN_NUMBER(nb_index)
BINARY_FUN_NUMBER(nb_matrix_multiply)
BINARY_FUN_NUMBER(nb_inplace_matrix_multiply)

static PyNumberMethods as_number_wrappers = {
        nb_add,                /*nb_add*/
        nb_subtract,           /*nb_subtract*/
        nb_multiply,           /*nb_multiply*/
        nb_remainder,          /*nb_remainder*/
        nb_divmod,             /*nb_divmod*/
        nb_power,              /*nb_power*/
        nb_negative,           /*nb_negative*/
        nb_positive,           /*np_positive*/
        nb_absolute,           /*nb_absolute*/
        nb_bool,               /*nb_bool*/
        nb_invert,             /*nb_invert*/
        nb_lshift,             /*nb_lshift*/
        nb_rshift,             /*nb_rshift*/
        nb_and,                /*nb_and*/
        nb_xor,                /*nb_xor*/
        nb_or,                 /*nb_or*/
        nb_int,                /*nb_int*/
        0,                     /*nb_reserved*/
        nb_float,              /*nb_float*/
        nb_inplace_add,        /* nb_inplace_add */
        nb_inplace_subtract,   /* nb_inplace_subtract */
        nb_inplace_multiply,   /* nb_inplace_multiply */
        nb_inplace_remainder,  /* nb_inplace_remainder */
        nb_inplace_power,      /* nb_inplace_power */
        nb_inplace_lshift,     /* nb_inplace_lshift */
        nb_inplace_rshift,     /* nb_inplace_rshift */
        nb_inplace_and,        /* nb_inplace_and */
        nb_inplace_xor,        /* nb_inplace_xor */
        nb_inplace_or,         /* nb_inplace_or */
        nb_floor_divide,       /* nb_floor_divide */
        nb_true_divide,        /* nb_true_divide */
        nb_inplace_floor_divide, /* nb_inplace_floor_divide */
        nb_inplace_true_divide,  /* nb_inplace_true_divide */
        nb_index,              /* nb_index */
        nb_matrix_multiply,    /* nb_matrix_multiply */
        nb_inplace_matrix_multiply /* nb_inplace_matrix_multiply */
};

PyTypeObject WrapperType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0) WrapperTypeName, /* tp_name */
        offsetof(Wrapper, concrete) + sizeof(PyObject *) * 2,   /* tp_basicsize */
        0,                                          /* tp_itemsize */
        (destructor) wrapper_dealloc,               /* tp_dealloc */
        0,                                          /* tp_vectorcall_offset TODO? */
        0,                                          /* tp_getattr */
        0,                                          /* tp_setattr */
        0,                                          /* tp_as_async */
        tp_repr,                                    /* tp_repr */
        &as_number_wrappers,                        /* tp_as_number */
        0,                                          /* tp_as_sequence */
        0,                                          /* tp_as_mapping */
        0,                                          /* tp_hash */
        tp_call,                                    /* tp_call */
        tp_str,                                     /* tp_str */
        tp_getattro,                                /* tp_getattro */
        tp_setattro,                                /* tp_setattro */
        0,                                          /* tp_as_buffer */
        0,                                          /* tp_flags */
        0,                                          /* tp_doc */
        0,                                          /* tp_traverse */
        0,                                          /* tp_clear */
        tp_richcompare,                             /* tp_richcompare */
        0,                                          /* tp_weaklistoffset */
        0,                                          /* tp_iter */
        0,                                          /* tp_iternext */
        0,                                          /* tp_methods (INITIALIZE IN RUNTIME) */
        0,                                          /* tp_members (INITIALIZE IN RUNTIME) */
        0,                                          /* tp_getset */
        0,                                          /* tp_base */
        0,                                          /* tp_dict */
        0,                                          /* tp_descr_get */
        0,                                          /* tp_descr_set */
        0,                                          /* tp_dictoffset */
        0,                                          /* tp_init */
        0,                                          /* tp_alloc */
        0,                                          /* tp_new (INITIALIZE IN RUNTIME?) */
        PyObject_Free,                              /* tp_free */
};

int
is_wrapped(PyObject *obj) {
    if (!obj)
        return 0;
    return obj->ob_type == &WrapperType;
}

PyObject *
wrap_concrete_object(PyObject *obj) {
    if (!obj)
        return NULL;
    if (obj == Py_NotImplemented)
        return Py_NotImplemented;
    Wrapper *result = (Wrapper *) _PyObject_New(&WrapperType);
    result->concrete = obj;
    Py_INCREF(obj);
    result->ob_base.ob_type = &WrapperType;
    return (PyObject *) result;
}

PyObject *
unwrap(PyObject *obj) {
    if (obj == NULL)
        return NULL;

    if (!is_wrapped(obj))
        return obj;

    Wrapper *obj_as_wrapper = (Wrapper *) obj;
    return obj_as_wrapper->concrete;
}