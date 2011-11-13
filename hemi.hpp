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
extern "C" PyObject * Context_Object(Context *self, PyObject *args);
extern "C" PyObject * Context_Function(Context *self, PyObject *args);
extern "C" PyObject * Context_getlocals(Context *self, void *closure);

static PyMethodDef Context_methods[] = {
    {(char *)"eval", (PyCFunction)Context_eval, METH_VARARGS,
     (char *)"Eval source in context"
    },
    {(char *)"Object", (PyCFunction)Context_Object, METH_NOARGS,
     (char *)"Create Javascript object"
    },
    {(char *)"Function", (PyCFunction)Context_Function, METH_VARARGS,
     (char *)"Create Javascript function"
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
} ObjectWrapper;

extern "C" void ObjectWrapper_dealloc(ObjectWrapper *self);
extern "C" PyObject * ObjectWrapper_getitem(ObjectWrapper *self, PyObject *item);
extern "C" int ObjectWrapper_setitem(ObjectWrapper *self, PyObject *item, PyObject *value);
extern "C" PyObject * ObjectWrapper_getattr(ObjectWrapper *self, PyObject *name);
extern "C" int ObjectWrapper_setattr(ObjectWrapper *self, PyObject *name, PyObject *value);

static PyMappingMethods ObjectWrapper_as_mapping = {
    0,
    (binaryfunc)ObjectWrapper_getitem,
    (objobjargproc)ObjectWrapper_setitem,
};

static PyTypeObject ObjectWrapperType = {
    PyObject_HEAD_INIT(NULL)
    0,                            /* ob_size */
    "hemi.Object",                /* tp_name */
    sizeof(ObjectWrapper),               /* tp_basicsize */
    0,                            /* tp_itemsize */
    (destructor)ObjectWrapper_dealloc,   /* tp_dealloc */
    0,                            /* tp_print */
    0,                            /* tp_getattr */
    0,                            /* tp_setattr */
    0,                            /* tp_compare */
    0,                            /* tp_repr */
    0,                            /* tp_as_number */
    0,                            /* tp_as_sequence */
    &ObjectWrapper_as_mapping,           /* tp_as_mapping */
    0,                            /* tp_hash */
    0,                            /* tp_call */
    0,                            /* tp_str */
    (getattrofunc)ObjectWrapper_getattr, /* tp_getattro */
    (setattrofunc)ObjectWrapper_setattr, /* tp_setattro */
    0,                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,           /* tp_flags */
    "Javascript object",          /* tp_doc */
};

extern "C" PyObject * FunctionWrapper_call(ObjectWrapper *self, PyObject *args, PyObject *kw);

static PyTypeObject FunctionWrapperType = {
    PyObject_HEAD_INIT(NULL)
    0,                            /* ob_size */
    "hemi.Function",              /* tp_name */
    sizeof(ObjectWrapper),        /* tp_basicsize */
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
    (ternaryfunc)FunctionWrapper_call,   /* tp_call */
    0,                            /* tp_str */
    0,                            /* tp_getattro */
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

typedef struct {
    const char *name;
    PyObject *type;
} supported_error_type;

PyObject *Error = PyErr_NewException((char *)"hemi.Error", PyExc_SyntaxError, NULL);

supported_error_type supported_errors[] = {
    {"Error", Error},
    {"RangeError", PyErr_NewException((char *)"hemi.RangeError", Error, NULL)},
    {"ReferenceError", PyErr_NewException((char *)"hemi.ReferenceError", Error, NULL)},
    {"SyntaxError", PyErr_NewException((char *)"hemi.SyntaxError", Error, NULL)},
    {"TypeError", PyErr_NewException((char *)"hemi.TypeError", Error, NULL)},
    {NULL} /* Sentinel */
};

void set_exception(v8::TryCatch &trycatch);

v8::Handle<v8::Value> unwrap(PyObject *py);

class UnwrapError {
protected:
    PyObject *m_object;
public:
    UnwrapError(PyObject *object);

    void set_exception();
    PyObject * get_message();
};

PyObject * wrap(v8::Handle<v8::Context> context, v8::Handle<v8::Object> parent, v8::Handle<v8::Value> value);
PyObject * wrap_primitive(v8::Handle<v8::Value> value);

extern "C" PyObject * pythonify(PyObject *self, PyObject *args);

static PyMethodDef module_methods[] = {
    {(char *)"pythonify", (PyCFunction)pythonify, METH_VARARGS,
     (char *)"Recursively turn wrapped Javascript objects into lists and dicts"
    },
    {NULL}  /* Sentinel */
};
