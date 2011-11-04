// -*- c-style: cc-mode -*-

#include <Python.h>
#include <v8.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    v8::Persistent<v8::Context> context;
} Context;

extern "C" void Context_dealloc(Context* self) {
    self->context.Dispose();

    self->ob_type->tp_free((PyObject *)self);
}

extern "C" int Context_init(Context *self, PyObject *args, PyObject *kwds) {
    using namespace v8;

    HandleScope handle_scope;

    self->context = v8::Context::New();

    v8::Context::Scope context_scope(self->context);

    PyObject* u_source;

    if(!PyArg_ParseTuple(args, "U", &u_source))
        return -1;

    PyObject* utf8_source = PyUnicode_AsUTF8String(u_source);

    Handle<String> source = String::New(PyString_AS_STRING(utf8_source));

    Py_DECREF(utf8_source);

    TryCatch trycatch;

    Handle<Script> script = Script::Compile(source);

    if(script.IsEmpty()) {
        Handle<Value> exception = trycatch.Exception();

        String::AsciiValue exception_str(exception);

        PyErr_SetString(PyExc_Exception, *exception_str);

        return -1;
    }

    script->Run();

    return 0;
}

extern "C" PyObject* Context_getattr(Context *self, PyObject *name) {
    using namespace v8;

    char *_name = PyString_AsString(name);

    HandleScope handle_scope;

    v8::Context::Scope context_scope(self->context);

    Handle<Value> result = self->context->Global()->Get(String::New("s"));

    String::AsciiValue ascii(result);

    //return Py_BuildValue("s", *ascii);

    if(result.IsEmpty()) {
        return PyObject_GenericGetAttr((PyObject *)self, name);
    }

    return Py_BuildValue("d", result->NumberValue());

    return NULL;
}

static PyTypeObject ContextType = {
    PyObject_HEAD_INIT(NULL)
    0,                           /* ob_size */
    "hemi.Context",              /* tp_name */
    sizeof(Context),             /* tp_basicsize */
    0,                           /* tp_itemsize */
    (destructor)Context_dealloc, /* tp_dealloc */
    0,                           /* tp_print */
    0,                           /* tp_getattr */
    0,                           /* tp_setattr */
    0,                           /* tp_compare */
    0,                           /* tp_repr */
    0,                           /* tp_as_number */
    0,                           /* tp_as_sequence */
    0,                           /* tp_as_mapping */
    0,                           /* tp_hash */
    0,                           /* tp_call */
    0,                           /* tp_str */
    (getattrofunc)Context_getattr, /* tp_getattro */
    0,                           /* tp_setattro */
    0,                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,          /* tp_flags */
    "Interpreter context",       /* tp_doc */
    0,		                     /* tp_traverse */
    0,		                     /* tp_clear */
    0,		                     /* tp_richcompare */
    0,		                     /* tp_weaklistoffset */
    0,		                     /* tp_iter */
    0,		                     /* tp_iternext */
    0,                           /* tp_methods */
    0,                           /* tp_members */
    0,                           /* tp_getset */
    0,                           /* tp_base */
    0,                           /* tp_dict */
    0,                           /* tp_descr_get */
    0,                           /* tp_descr_set */
    0,                           /* tp_dictoffset */
    (initproc)Context_init,      /* tp_init */
    0,                           /* tp_alloc */
};

static PyMethodDef module_methods[] = {
    {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
inithemi(void) 
{
    PyObject* m;

    ContextType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&ContextType) < 0)
        return;

    m = Py_InitModule3("hemi", module_methods,
                       "Lightweight V8 wrapper.");

    Py_INCREF(&ContextType);
    PyModule_AddObject(m, "Context", (PyObject *)&ContextType);
}
