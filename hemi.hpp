// -*- c-file-style: "cc-mode" -*-

#include <Python.h>
#include <v8.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    v8::Persistent<v8::Context> context;
} Context;

extern "C" PyObject * Context_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
extern "C" void Context_dealloc(Context *self);
extern "C" PyObject * Context_eval(Context *self, PyObject *args);
extern "C" PyObject * Context_getlocals(Context *self, void *closure);

static PyMethodDef Context_methods[] = {
    {(char *)"eval", (PyCFunction)Context_eval, METH_VARARGS,
     (char *)"Eval source in context"
    },
    {NULL} /* Sentinel */
};

static PyGetSetDef Context_getseters[] = {
    {(char *)"locals",
     (getter)Context_getlocals, NULL,
     (char *)"Context locals",
     NULL},
    {NULL} /* Sentinel */
};

static PyTypeObject ContextType = {
    PyObject_HEAD_INIT(NULL)
    0,                             /* ob_size */
    "hemi.Context",                /* tp_name */
    sizeof(Context),               /* tp_basicsize */
    0,                             /* tp_itemsize */
    (destructor)Context_dealloc,   /* tp_dealloc */
    0,                             /* tp_print */
    0,                             /* tp_getattr */
    0,                             /* tp_setattr */
    0,                             /* tp_compare */
    0,                             /* tp_repr */
    0,                             /* tp_as_number */
    0,                             /* tp_as_sequence */
    0,                             /* tp_as_mapping */
    0,                             /* tp_hash */
    0,                             /* tp_call */
    0,                             /* tp_str */
    0,                             /* tp_getattro */
    0,                             /* tp_setattro */
    0,                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,            /* tp_flags */
    "Interpreter context",         /* tp_doc */
    0,		                       /* tp_traverse */
    0,		                       /* tp_clear */
    0,		                       /* tp_richcompare */
    0,		                       /* tp_weaklistoffset */
    0,		                       /* tp_iter */
    0,		                       /* tp_iternext */
    Context_methods,               /* tp_methods */
    0,                             /* tp_members */
    Context_getseters,             /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    0,                             /* tp_init */
    0,                             /* tp_alloc */
    Context_new,                   /* tp_new */
};

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    v8::Persistent<v8::Context> context;
    v8::Persistent<v8::Object> parent;
    v8::Persistent<v8::Object> object;
} Object;

extern "C" void Object_dealloc(Object *self);
extern "C" PyObject * Object_getitem(Object *self, PyObject *item);
extern "C" PyObject * Object_getattr(Object *self, PyObject *name);

static PyMappingMethods Object_as_mapping = {
    0,
    (binaryfunc)Object_getitem,
};

static PyTypeObject ObjectType = {
    PyObject_HEAD_INIT(NULL)
    0,                            /* ob_size */
    "hemi.Object",                /* tp_name */
    sizeof(Object),               /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)Object_dealloc,   /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_compare */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    &Object_as_mapping,           /* tp_as_mapping */
    0,                            /* tp_hash */
    0,                            /* tp_call */
    0,                            /* tp_str */
    (getattrofunc)Object_getattr, /* tp_getattro */
    0,                            /* tp_setattro */
    0,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    "Javascript object",          /* tp_doc */
};

extern "C" PyObject * Function_call(Object *self, PyObject *args, PyObject *kw);

static PyTypeObject FunctionType = {
    PyObject_HEAD_INIT(NULL)
    0,                            /* ob_size */
    "hemi.Function",              /* tp_name */
    sizeof(Object),               /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)Object_dealloc,   /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_compare */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    &Object_as_mapping,           /* tp_as_mapping */
    0,                            /* tp_hash */
    (ternaryfunc)Function_call,   /* tp_call */
    0,                            /* tp_str */
    (getattrofunc)Object_getattr, /* tp_getattro */
    0,                            /* tp_setattro */
    0,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    "Javascript function",        /* tp_doc */
};

static PyTypeObject UndefinedType = {
    PyObject_HEAD_INIT(NULL)
    0,                            /* ob_size */
    "hemi.Undefined",             /* tp_name */
    sizeof(PyObject),             /* tp_basicsize */
    0,                            /* tp_itemsize */
    0,                            /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_compare */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    0,                            /* tp_as_mapping */
    0,                            /* tp_hash */
    0,                            /* tp_call */
    0,                            /* tp_str */
    0,                            /* tp_getattro */
    0,                            /* tp_setattro */
    0,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    "Javascript undefined",      /* tp_doc */
};

PyObject *Undefined;

v8::Handle<v8::Value> py_to_json(PyObject *py);

PyObject* wrap(v8::Handle<v8::Context> context, v8::Handle<v8::Object> parent, v8::Handle<v8::Value> value);

static PyMethodDef module_methods[] = {
    {NULL}  /* Sentinel */
};
