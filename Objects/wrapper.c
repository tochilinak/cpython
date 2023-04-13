#include <stddef.h>
#include "wrapper.h"
#include "objimpl.h"

static PyObject *
get_orig(PyObject *obj) {
    if (!obj)
        return NULL;
    if (strcmp(obj->ob_type->tp_name, WrapperTypeName) == 0)
        return ((Wrapper *) obj)->concrete;
    return obj;
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

    //printf("calling %s ", "tp_getattro");
    //PyObject_Print(other, stdout, 0);
    //printf("\n");
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
    //printf("calling %s on %p\n", "tp_repr", self);
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
    //printf("calling %s on %p\n", "tp_str", self);
    PyObject *concrete_self = get_orig(self);
    if (concrete_self->ob_type->tp_str == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    return concrete_self->ob_type->tp_str(concrete_self);
}

static PyObject *
tp_richcompare(PyObject *self, PyObject *other, int op) {
    //printf("calling %s %d on %p\n", "tp_richcompare", op, self);
    PyObject *concrete_self = get_orig(self);
    PyObject *concrete_other = get_orig(other);
    if (concrete_self->ob_type->tp_richcompare == 0) {
        PyErr_SetString(PyExc_TypeError, "no richcompare");
        return 0;
    }
    return wrap_concrete_object(concrete_self->ob_type->tp_richcompare(concrete_self, concrete_other, op));
}

static PyObject *tp_call(PyObject *self, PyObject *o1, PyObject *o2) {
    //printf("calling %s\n", "tp_call");
    PyObject *concrete_self = get_orig(self);
    for (int i = 0; i < PyTuple_GET_SIZE(o1); i++) {
        PyTuple_SET_ITEM(o1, i, get_orig(PyTuple_GetItem(o1, i)));
    }
    // TODO: unwrap o2
    //PyObject *concrete_o2 = get_orig(o2);
    if (concrete_self->ob_type->tp_call == 0) {
        PyErr_SetString(PyExc_TypeError, "no func");
        return 0;
    }
    return wrap_concrete_object(concrete_self->ob_type->tp_call(concrete_self, o1, o2));
}

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
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as->func(concrete_self)); \
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
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as->func(concrete_self, concrete_other)); \
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
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as->func(concrete_self, concrete_o1, concrete_o2)); \
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
                                        return wrap_concrete_object(concrete_self->ob_type->tp_as->func(concrete_self, i)); \
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
BINARY_FUN_AS(nb_subtract, tp_as_number)
BINARY_FUN_AS(nb_multiply, tp_as_number)
BINARY_FUN_AS(nb_remainder, tp_as_number)
BINARY_FUN_AS(nb_divmod, tp_as_number)
TERNARY_FUN_AS(nb_power, tp_as_number)
UNARY_FUN_AS(nb_negative, tp_as_number)
UNARY_FUN_AS(nb_positive, tp_as_number)
UNARY_FUN_AS(nb_absolute, tp_as_number)
INQUIRY_NUMBER(nb_bool)
UNARY_FUN_AS(nb_invert, tp_as_number)
BINARY_FUN_AS(nb_lshift, tp_as_number)
BINARY_FUN_AS(nb_rshift, tp_as_number)
BINARY_FUN_AS(nb_and, tp_as_number)
BINARY_FUN_AS(nb_xor, tp_as_number)
BINARY_FUN_AS(nb_or, tp_as_number)
UNARY_FUN_AS(nb_int, tp_as_number)
UNARY_FUN_AS(nb_float, tp_as_number)
BINARY_FUN_AS(nb_inplace_add, tp_as_number)
BINARY_FUN_AS(nb_inplace_subtract, tp_as_number)
BINARY_FUN_AS(nb_inplace_multiply, tp_as_number)
BINARY_FUN_AS(nb_inplace_remainder, tp_as_number)
TERNARY_FUN_AS(nb_inplace_power, tp_as_number)
BINARY_FUN_AS(nb_inplace_lshift, tp_as_number)
BINARY_FUN_AS(nb_inplace_rshift, tp_as_number)
BINARY_FUN_AS(nb_inplace_and, tp_as_number)
BINARY_FUN_AS(nb_inplace_xor, tp_as_number)
BINARY_FUN_AS(nb_inplace_or, tp_as_number)
BINARY_FUN_AS(nb_floor_divide, tp_as_number)
BINARY_FUN_AS(nb_true_divide, tp_as_number)
BINARY_FUN_AS(nb_inplace_floor_divide, tp_as_number)
BINARY_FUN_AS(nb_inplace_true_divide, tp_as_number)
UNARY_FUN_AS(nb_index, tp_as_number)
BINARY_FUN_AS(nb_matrix_multiply, tp_as_number)
BINARY_FUN_AS(nb_inplace_matrix_multiply, tp_as_number)

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

LEN_FUN_AS(sq_length, tp_as_sequence)
BINARY_FUN_AS(sq_concat, tp_as_sequence)
SIZEARG_FUN_AS(sq_repeat, tp_as_sequence)
SIZEARG_FUN_AS(sq_item, tp_as_sequence)
SIZEOBJARG_AS(sq_ass_item, tp_as_sequence)
OBJOBJ_AS(sq_contains, tp_as_sequence)
BINARY_FUN_AS(sq_inplace_concat, tp_as_sequence)
SIZEARG_FUN_AS(sq_inplace_repeat, tp_as_sequence)

static PySequenceMethods as_sequence_wrappers = {
        (lenfunc)sq_length,                       /* sq_length */
        (binaryfunc)sq_concat,                    /* sq_concat */
        (ssizeargfunc)sq_repeat,                  /* sq_repeat */
        (ssizeargfunc)sq_item,                    /* sq_item */
        0,                                        /* sq_slice */
        (ssizeobjargproc)sq_ass_item,             /* sq_ass_item */
        0,                                        /* sq_ass_slice */
        (objobjproc)sq_contains,                  /* sq_contains */
        (binaryfunc)sq_inplace_concat,            /* sq_inplace_concat */
        (ssizeargfunc)sq_inplace_repeat,          /* sq_inplace_repeat */
};

LEN_FUN_AS(mp_length, tp_as_mapping)
BINARY_FUN_AS(mp_subscript, tp_as_mapping)
OBJOBJARG_AS(mp_ass_subscript, tp_as_mapping)

static PyMappingMethods as_mapping_wrappers = {
        (lenfunc)mp_length,               /*mp_length*/
        (binaryfunc)mp_subscript,         /*mp_subscript*/
        (objobjargproc) mp_ass_subscript, /*mp_ass_subscript*/
};

PyTypeObject WrapperType = {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        WrapperTypeName,                            /* tp_name */
        offsetof(Wrapper, concrete) + sizeof(PyObject *) * 2,   /* tp_basicsize */
        0,                                          /* tp_itemsize */
        (destructor) wrapper_dealloc,               /* tp_dealloc */
        0,                                          /* tp_vectorcall_offset */
        0,                                          /* tp_getattr */
        0,                                          /* tp_setattr */
        0,                                          /* tp_as_async */
        tp_repr,                                    /* tp_repr */
        &as_number_wrappers,                        /* tp_as_number */
        &as_sequence_wrappers,                      /* tp_as_sequence */
        &as_mapping_wrappers,                       /* tp_as_mapping */
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
        0,                                          /* tp_methods */
        0,                                          /* tp_members */
        0,                                          /* tp_getset */
        0,                                          /* tp_base */
        0,                                          /* tp_dict */
        0,                                          /* tp_descr_get */
        0,                                          /* tp_descr_set */
        0,                                          /* tp_dictoffset */
        0,                                          /* tp_init */
        0,                                          /* tp_alloc */
        0,                                          /* tp_new */
        PyObject_Free,                              /* tp_free */
};

int
is_wrapped(PyObject *obj) {
    if (!obj)
        return 0;
    return strcmp(obj->ob_type->tp_name, WrapperTypeName) == 0;
}

#define SET_IF_EXISTS(func) \
do { \
    if (Py_TYPE(concrete)->func) \
        wrapper_type->func = func; \
    else \
        wrapper_type->func = 0; \
} while (0)

PyTypeObject *
create_new_wrapper_type(PyObject *concrete) {
    PyTypeObject *wrapper_type = (PyTypeObject *) PyObject_GC_New(PyTypeObject, &PyType_Type);
    wrapper_type->tp_name = WrapperTypeName;
    wrapper_type->tp_basicsize = offsetof(Wrapper, concrete) + sizeof(PyObject *) * 2;
    wrapper_type->tp_itemsize = 0;
    wrapper_type->tp_dealloc = (destructor)wrapper_dealloc;
    wrapper_type->tp_getattr = 0;
    wrapper_type->tp_setattr = 0;
    wrapper_type->tp_as_async = 0;  // TODO?
    SET_IF_EXISTS(tp_repr);
}

PyObject *
wrap_concrete_object(PyObject *obj) {
    if (!obj)
        return NULL;
    if (obj == Py_NotImplemented)
        return Py_NotImplemented;
    if (is_wrapped(obj))
        return obj;
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
    PyObject *result = obj_as_wrapper->concrete;
    Py_DECREF(obj_as_wrapper);
    return result;
}