// -*- c-file-style: "cc-mode" -*-

#include "hemi.hpp"

extern "C" void Context_dealloc(Context* self) {
    self->context.Dispose();

    self->ob_type->tp_free((PyObject *)self);
}

extern "C" PyObject * Context_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
    Context *self = (Context *)subtype->tp_alloc(subtype, 0);

    if(self != NULL) {
        //HandleScope handle_scope;

        self->context = v8::Context::New();

        if(self->context.IsEmpty()) {
            Py_DECREF(self);
            return NULL;
        }
    }

    return (PyObject *)self;
}

extern "C" PyObject * Context_eval(Context *self, PyObject *args) {
    using namespace v8;

    char* _source;

    if(!PyArg_ParseTuple(args, "es", "utf-8", &_source))
        return NULL;

    HandleScope handle_scope;

    v8::Context::Scope context_scope(self->context);

    Handle<String> source = String::New(_source);

    PyMem_Free(_source);

    TryCatch trycatch;

    Handle<Script> script = Script::Compile(source);

    if(script.IsEmpty()) {
        Handle<Value> exception = trycatch.Exception();

        String::AsciiValue exception_str(exception);

        PyErr_SetString(PyExc_Exception, *exception_str);

        return NULL;
    }

    Handle<Value> result = script->Run();

    if(result.IsEmpty()) {
        Handle<Value> exception = trycatch.Exception();

        String::AsciiValue exception_str(exception);

        PyErr_SetString(PyExc_Exception, *exception_str);

        return NULL;
    }

    PyObject * py_result = wrap(self->context, Handle<v8::Object>(), result);

    return py_result;
}

extern "C" PyObject * Context_getlocals(Context *self, void *closure) {
    using namespace v8;

    v8::Context::Scope context_scope(self->context);

    HandleScope handle_scope;

    Handle<Value> object(self->context->Global());

    PyObject *wrapped = wrap(self->context, Handle<v8::Object>(), object);

    return wrapped;
}

extern "C" void Object_dealloc(Object* self) {
    v8::HandleScope handle_scope;

    self->object->DeleteHiddenValue(v8::String::New("hemi::wrapper"));

    self->object.Dispose();
    self->parent.Dispose();
    self->context.Dispose();

    self->ob_type->tp_free((PyObject *)self);
}

extern "C" PyObject * Object_getitem(Object *self, PyObject *item) {
    using namespace v8;

    if(PyUnicode_Check(item)) {
        PyObject *_item = PyUnicode_AsUTF8String(item);

        v8::Context::Scope context_scope(self->context);

        Handle<Value> result = self->object->GetRealNamedProperty(String::New(PyString_AS_STRING(_item)));

        Py_DECREF(_item);

        if(!result.IsEmpty())
            return wrap(self->context, self->object, result);
    }

    PyErr_SetObject(PyExc_KeyError, item);

    return NULL;
};

extern "C" PyObject * Object_getattr(Object *self, PyObject *name) {
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

    // Have to reset exception raised by GetAttr
    PyErr_Clear();

    return wrap(self->context, self->object, result);
};

extern "C" PyObject * Function_call(Object *self, PyObject *args, PyObject *kw) {
    int argc = (int)PySequence_Size(args);

    v8::HandleScope handle_scope;

    v8::Context::Scope context_scope(self->context);

    v8::Handle<v8::Value> argv[argc];

    try {
        for(int i = 0; i < argc; i++) {
            argv[i] = py_to_json(PySequence_GetItem(args, i));
        }
    } catch (PyObject *bad_object) {
        PyObject *repr = PyObject_Repr(bad_object);

        const char *repr_string;

        if(repr == NULL) {
            repr_string = "<unpresentable object>";
        } else {
            repr_string = PyString_AS_STRING(repr);
        }

        PyErr_Format(PyExc_TypeError, "cannot represent %s to javascript", repr_string);

        Py_XDECREF(repr);

        return NULL;
    }

    v8::Handle<v8::Value> result = self->object.As<v8::Function>()->Call(self->parent, argc, argv);

    return wrap(self->context, v8::Handle<v8::Object>(), result);
};

v8::Handle<v8::Value> py_to_json(PyObject *py) {
    if(py == Py_True)
        return v8::True();

    if(py == Py_False)
        return v8::False();

    if(py == Py_None)
        return v8::Null();

    if(PyInt_Check(py)) {
        long value = PyInt_AS_LONG(py);

        if(sizeof(long) == sizeof(int32_t)) {
            return v8::Integer::New(value);
        } else {
            if(-value < 1ll << 16 && value < 1ll << 16)
                return v8::Integer::New(value);

            if(value > 0ll && value < 1ll << 32)
                return v8::Integer::NewFromUnsigned(value);
        }
    }

    if(PyFloat_Check(py))
        return v8::Number::New(PyFloat_AS_DOUBLE(py));

    if(PyString_Check(py)) {
        PyObject *unicode = PyUnicode_FromObject(py);
        PyObject *py_string = PyUnicode_AsUTF8String(unicode);

        v8::Handle<v8::String> js_string = v8::String::New(PyString_AS_STRING(py_string));

        Py_DECREF(py_string);
        Py_DECREF(unicode);

        return js_string;
    }

    if(PyUnicode_Check(py)) {
        PyObject *py_string = PyUnicode_AsUTF8String(py);

        v8::Handle<v8::String> js_string = v8::String::New(PyString_AS_STRING(py_string));

        Py_DECREF(py_string);

        return js_string;
    }

    if(PyList_Check(py)) {
        uint32_t length = PyList_GET_SIZE(py);

        v8::Handle<v8::Array> array = v8::Array::New(length);

        for(uint32_t i = 0; i < length; i++) {
            v8::Handle<v8::Value> item = py_to_json(PyList_GET_ITEM(py, i));

            array->Set(i, item);
        }

        return array;
    }

    if(PyDict_Check(py)) {
        PyObject *key, *value;
        Py_ssize_t pos = 0;

        v8::Handle<v8::Object> object = v8::Object::New();

        while(PyDict_Next(py, &pos, &key, &value)) {
            v8::Handle<v8::Value> js_key = py_to_json(key);

            object->Set(js_key, py_to_json(value));
        }

        return object;
    }

    throw py;
};

PyObject * wrap(v8::Handle<v8::Context> context, v8::Handle<v8::Object> parent, v8::Handle<v8::Value> value) {
    if(value->IsInt32())
        return PyInt_FromLong(value->Int32Value());

    if(value->IsNumber())
        return PyFloat_FromDouble(value->NumberValue());

    if(value->IsBoolean())
        return PyBool_FromLong(value->BooleanValue());

    if(value->IsNull())
        Py_RETURN_NONE;

    if(value->IsUndefined()) {
        return Py_INCREF(Undefined), Undefined;
    }

    if(value->IsString()) {
        v8::String::Utf8Value utf_string(value);
        return PyUnicode_FromString(*utf_string);
    }

    Object *object;

    v8::Handle<v8::Value> wrapper = value->ToObject()->GetHiddenValue(v8::String::New("hemi::wrapper"));

    if(!wrapper.IsEmpty()) {
        object = (Object *)v8::External::Unwrap(wrapper);

        Py_INCREF(object);

        return (PyObject *)object;
    }

    if (value->IsFunction()) {
        object = PyObject_New(Object, &FunctionType);
    } else {
        object = PyObject_New(Object, &ObjectType);
    }

    object->context = v8::Persistent<v8::Context>::New(context);
    object->parent = v8::Persistent<v8::Object>::New(parent);
    object->object = v8::Persistent<v8::Object>::New(value.As<v8::Object>());

    object->object->SetHiddenValue(v8::String::New("hemi::wrapper"), v8::External::Wrap(object));

    return (PyObject *)object;
}

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
inithemi(void)
{
    PyObject* m;

    if (PyType_Ready(&ContextType) < 0)
        return;

    if (PyType_Ready(&ObjectType) < 0)
        return;

    if (PyType_Ready(&FunctionType) < 0)
        return;

    if (PyType_Ready(&UndefinedType) < 0)
        return;

    Undefined = PyObject_New(PyObject, &UndefinedType);

    m = Py_InitModule3("hemi", module_methods,
                       "Lightweight V8 wrapper.");

    Py_INCREF(&ContextType);
    PyModule_AddObject(m, "Context", (PyObject *)&ContextType);
}
