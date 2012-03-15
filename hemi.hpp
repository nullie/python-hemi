// -*- c-file-style: "cc-mode" -*-

#include <Python.h>
#include <v8.h>

using namespace v8;

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    v8::Persistent<v8::Context> context;
} ContextWrapper;

extern "C" PyObject * ContextWrapper_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds);
extern "C" void ContextWrapper_dealloc(ContextWrapper *self);
extern "C" PyObject * ContextWrapper_eval(ContextWrapper *self, PyObject *args);
extern "C" PyObject * ContextWrapper_Object(ContextWrapper *self, PyObject *args);
extern "C" PyObject * ContextWrapper_Function(ContextWrapper *self, PyObject *args);
extern "C" PyObject * ContextWrapper_getlocals(ContextWrapper *self, void *closure);

static PyMethodDef ContextWrapper_methods[] = {
    {(char *)"eval", (PyCFunction)ContextWrapper_eval, METH_VARARGS,
     (char *)"Eval source in context"
    },
    {(char *)"Object", (PyCFunction)ContextWrapper_Object, METH_NOARGS,
     (char *)"Create Javascript object"
    },
    {(char *)"Function", (PyCFunction)ContextWrapper_Function, METH_VARARGS,
     (char *)"Create Javascript function"
    },
    {NULL} /* Sentinel */
};

static PyGetSetDef ContextWrapper_getseters[] = {
    {(char *)"locals",
     (getter)ContextWrapper_getlocals, NULL,
     (char *)"Context locals",
     NULL},
    {NULL} /* Sentinel */
};

static PyTypeObject ContextWrapperType = {
    PyObject_HEAD_INIT(NULL)
    0,                             /* ob_size */
    (char *)"hemi.Context",                /* tp_name */
    sizeof(ContextWrapper),               /* tp_basicsize */
    0,                             /* tp_itemsize */
    (destructor)ContextWrapper_dealloc,   /* tp_dealloc */
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
    (char *)"Interpreter context",       /* tp_doc */
    0,		                       /* tp_traverse */
    0,		                       /* tp_clear */
    0,		                       /* tp_richcompare */
    0,		                       /* tp_weaklistoffset */
    0,		                       /* tp_iter */
    0,		                       /* tp_iternext */
    ContextWrapper_methods,               /* tp_methods */
    0,                             /* tp_members */
    ContextWrapper_getseters,             /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    0,                             /* tp_init */
    0,                             /* tp_alloc */
    ContextWrapper_new,                   /* tp_new */
};

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    Persistent<Context> context;
    Persistent<Object> parent;
    Persistent<Object> object;
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
    "hemi.undefined",             /* tp_name */
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

PyObject *Py_Undefined;

typedef struct {
    const char *name;
    PyObject *type;
} supported_error_type;

PyObject *Error = PyErr_NewException((char *)"hemi.Error", PyExc_SyntaxError, NULL);

supported_error_type supported_errors[] = {
    {"Error", Error},
    {"TypeError", PyErr_NewException((char *)"hemi.TypeError", Error, NULL)},
    {"RangeError", PyErr_NewException((char *)"hemi.RangeError", Error, NULL)},
    {"SyntaxError", PyErr_NewException((char *)"hemi.SyntaxError", Error, NULL)},
    {"ReferenceError", PyErr_NewException((char *)"hemi.ReferenceError", Error, NULL)},
    {"EvalError", PyErr_NewException((char *)"hemi.EvalError", Error, NULL)},
    {"URIError", PyErr_NewException((char *)"hemi.URIError", Error, NULL)},
    {NULL} /* Sentinel */
};

void set_exception(TryCatch &trycatch);

class UnwrapError {
protected:
    PyObject *m_object;
    PyObject *m_type;
    PyObject *m_value;
    PyObject *m_traceback;
public:
    UnwrapError(PyObject *object, bool exception = false);

    void set_exception();
    PyObject * get_message();
};

Handle<Value> unwrap(PyObject *py);

PyObject * wrap(Handle<Context> context, Handle<Object> parent, Handle<Value> value);
PyObject * pythonify_primitive(Handle<Value> value);

Handle<Value> wrap_pyobject(PyObject *object);
PyObject * unwrap_pyobject(Handle<Value> value);

extern "C" PyObject * Hemi_pythonify(PyObject *self, PyObject *args);

static PyMethodDef module_methods[] = {
    {(char *)"pythonify", (PyCFunction)Hemi_pythonify, METH_VARARGS,
     (char *)"Recursively turn wrapped Javascript objects into lists and dicts"
    },
    {NULL}  /* Sentinel */
};
