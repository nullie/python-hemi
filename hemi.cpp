#include <Python.h>
#include <v8.h>

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    v8::Persistent<v8::Context> context;
} Context;

static void Context_dealloc(Context* self) {
    self->context.Dispose();

    self->ob_type->tp_free((PyObject *)self);
}

static int Context_init(Context *self, PyObject *args, PyObject *kwds) {
    self->context = v8::Context::New();

    return 0;
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
    0,                           /* tp_getattro */
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
