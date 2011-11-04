// -*- c-file-style: "cc-mode" -*-

#include <Python.h>
#include <v8.h>

PyObject* wrap(v8::Handle<v8::Context> context, v8::Handle<v8::Value> value);

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

    char* _source;

    if(!PyArg_ParseTuple(args, "es", "utf-8", &_source))
        return -1;

    Handle<String> source = String::New(_source);

    PyMem_Free(_source);

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

extern "C" PyObject* Context_getglobals(Context *self, void *closure) {
    using namespace v8;

    v8::Context::Scope context_scope(self->context);

    HandleScope handle_scope;

    Handle<Value> object(self->context->Global());

    PyObject *wrapped = wrap(self->context, object);

    return wrapped;
}

static PyGetSetDef Context_getseters[] = {
    {"globals",
     (getter)Context_getglobals, NULL,
     "Context globals",
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
    0,                             /* tp_methods */
    0,                             /* tp_members */
    Context_getseters,             /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    (initproc)Context_init,        /* tp_init */
    0,                             /* tp_alloc */
};

typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    v8::Persistent<v8::Context> context;
    v8::Persistent<v8::Object> object;
} Object;

extern "C" void Object_dealloc(Object* self) {
    self->object.Dispose();

    self->ob_type->tp_free((PyObject *)self);
}

extern "C" PyObject* Object_getitem(Object *self, PyObject *item) {
    using namespace v8;

    if(PyUnicode_Check(item)) {
        PyObject *_item = PyUnicode_AsUTF8String(item);

        v8::Context::Scope context_scope(self->context);

        Handle<Value> result = self->object->GetRealNamedProperty(String::New(PyString_AS_STRING(_item)));

        Py_DECREF(_item);

        if(!result.IsEmpty())
            return Py_BuildValue("d", result->NumberValue());
    }

    PyErr_SetObject(PyExc_KeyError, item);
    return NULL;
};

extern "C" PyObject* Object_getattr(Object *self, PyObject *name) {
    using namespace v8;

    PyObject *value = PyObject_GenericGetAttr((PyObject *)self, name);

    if(value != NULL)
        return value;

    char *_name = PyString_AsString(name);

    HandleScope handle_scope;

    v8::Context::Scope context_scope(self->context);

    Handle<Value> result = self->object->GetRealNamedProperty(String::New(_name));

    if(result.IsEmpty()) {
        // Exception already set by GetAttr
        return NULL;
    }

    return Py_BuildValue("d", result->NumberValue());
};

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

PyObject* wrap(v8::Handle<v8::Context> context, v8::Handle<v8::Value> value) {
    PyObject *rv;

    if(value->IsNumber()) {
        rv = Py_BuildValue("d", value->NumberValue());
    } else {
        Object *object = PyObject_New(Object, &ObjectType);

        object->context = v8::Persistent<v8::Context>::New(context);
        object->object = v8::Persistent<v8::Object>::New(value.As<v8::Object>());

        rv = (PyObject *)object;
    }

    return rv;
}

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

    if (PyType_Ready(&ObjectType) < 0)
        return;

    m = Py_InitModule3("hemi", module_methods,
                       "Lightweight V8 wrapper.");

    Py_INCREF(&ContextType);
    PyModule_AddObject(m, "Context", (PyObject *)&ContextType);
}
