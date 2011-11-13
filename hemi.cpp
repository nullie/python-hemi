// -*- c-file-style: "cc-mode" -*-

#include "hemi.hpp"

extern "C" void ContextWrapper_dealloc(ContextWrapper* self) {
    self->context.Dispose();

    self->ob_type->tp_free((PyObject *)self);
}

extern "C" PyObject * ContextWrapper_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
    ContextWrapper *self = (ContextWrapper *)subtype->tp_alloc(subtype, 0);

    if(self != NULL) {
        self->context = Context::New();

        if(self->context.IsEmpty()) {
            Py_DECREF(self);
            return NULL;
        }
    }

    return (PyObject *)self;
}

extern "C" PyObject * ContextWrapper_eval(ContextWrapper *self, PyObject *args) {
    using namespace v8;

    char* _source;

    if(!PyArg_ParseTuple(args, "es", "utf-8", &_source))
        return NULL;

    HandleScope handle_scope;

    Context::Scope context_scope(self->context);

    Handle<String> source = String::New(_source);

    PyMem_Free(_source);

    TryCatch trycatch;

    Handle<Script> script = Script::Compile(source);

    if(script.IsEmpty()) {
        set_exception(trycatch);

        return NULL;
    }

    Handle<Value> result = script->Run();

    if(result.IsEmpty()) {
        set_exception(trycatch);

        return NULL;
    }

    PyObject *py_result = wrap(self->context, Handle<Object>(), result);

    return py_result;
}

extern "C" PyObject * ContextWrapper_Object(ContextWrapper *self, PyObject *args) {
    HandleScope handle_scope;

    Context::Scope context_scope(self->context);

    Handle<Object> object = Object::New();

    ObjectWrapper *wrapper = PyObject_New(ObjectWrapper, &ObjectWrapperType);

    wrapper->context = Persistent<Context>::New(self->context);
    wrapper->object = Persistent<Object>::New(object);

    return (PyObject *)wrapper;
}

Handle<Value> FunctionWrapper_callback(const Arguments &args) {
    PyObject *callback = (PyObject *)External::Unwrap(args.Data());

    Handle<Context> context = Context::GetCurrent();

    int args_length = args.Length();

    PyObject *py_args = PyList_New(1 + args_length);

    PyList_SetItem(py_args, 0, wrap(context, Handle<Object>(), args.This()));

    for(int i = 0; i < args_length; i++) {
        PyList_SetItem(py_args, i + 1, wrap(context, Handle<Object>(), args[i]));
    }

    PyObject *rv = PyObject_Call(callback, py_args, NULL);

    PyObject *message;
    Handle<String> js_message;
    Handle<Value> exception;

    if(rv == NULL) {
        PyObject *ptype, *pvalue, *ptraceback;

        PyErr_Fetch(&ptype, &pvalue, &ptraceback);

        Py_XDECREF(ptraceback);

        PyObject *format = PyString_FromString("%s: %s");

        PyObject *type_name = PyObject_GetAttrString(ptype, "__name__");
        Py_DECREF(ptype);

        PyObject *mm = Py_BuildValue("OO", type_name, pvalue);

        Py_XDECREF(pvalue);

        Py_DECREF(type_name);

        message = PyUnicode_Format(format, mm);

        Py_DECREF(format);

        exception = Exception::Error(unwrap(message).As<String>());

        Py_DECREF(message);

        ThrowException(exception);

        return Handle<Value>();
    }

    Handle<Value> js_rv;

    try {
        js_rv = unwrap(rv);
    } catch(UnwrapError error) {
        message = error.get_message();

        exception = Exception::TypeError(unwrap(message).As<String>());

        Py_DECREF(message);

        ThrowException(exception);

        return Handle<Value>();
    }

    return js_rv;
}

void FunctionWrapper_dispose(Persistent<Value> object, void *callback) {
    Py_DECREF(callback);
}

extern "C" PyObject * ContextWrapper_Function(ContextWrapper *self, PyObject *args) {
    PyObject *callable;

    if(!PyArg_ParseTuple(args, "O", &callable))
        return NULL;

    if(!PyCallable_Check(callable)) {
        return NULL;
    }

    HandleScope handle_scope;

    Context::Scope context_scope(self->context);

    Handle<Function> function = FunctionTemplate::New(FunctionWrapper_callback, External::Wrap(callable))->GetFunction();

    Py_INCREF(callable);

    Persistent<Function>::New(function).MakeWeak(callable, FunctionWrapper_dispose);

    ObjectWrapper *wrapper = PyObject_New(ObjectWrapper, &FunctionWrapperType);

    wrapper->context = Persistent<Context>::New(self->context);
    wrapper->parent = Persistent<Object>();
    wrapper->object = Persistent<Object>::New(function);

    return (PyObject *)wrapper;
}

extern "C" PyObject * ContextWrapper_getlocals(ContextWrapper *self, void *closure) {
    Context::Scope context_scope(self->context);

    HandleScope handle_scope;

    Handle<Value> object(self->context->Global());

    PyObject *wrapped = wrap(self->context, Handle<Object>(), object);

    return wrapped;
}

extern "C" void ObjectWrapper_dealloc(ObjectWrapper* self) {
    HandleScope handle_scope;

    self->object.Dispose();
    self->parent.Dispose();
    self->context.Dispose();

    self->ob_type->tp_free((PyObject *)self);
}

extern "C" PyObject * ObjectWrapper_getitem(ObjectWrapper *self, PyObject *item) {
    HandleScope handle_scope;

    Context::Scope context_scope(self->context);

    Handle<Value> key = unwrap(item);

    Handle<Value> result = self->object->GetRealNamedProperty(key.As<String>());

    if(!result.IsEmpty())
        return wrap(self->context, self->object, result);

    PyErr_SetObject(PyExc_KeyError, item);

    return NULL;
};

extern "C" int ObjectWrapper_setitem(ObjectWrapper *self, PyObject *item, PyObject *value) {
    HandleScope handle_scope;

    Context::Scope context_scope(self->context);

    Handle<Value> key, js_value;

    try {
        key = unwrap(item);

        if(value)
            js_value = unwrap(value);
    } catch(UnwrapError error) {
        error.set_exception();
        return -1;
    }

    TryCatch trycatch;

    bool ok;

    if(value == NULL) {
        ok = self->object->Delete(key->ToString());
    } else {
        ok = self->object->Set(key, js_value);
    }

    if(!ok) {
        set_exception(trycatch);

        return -1;
    }

    return 0;
}

extern "C" PyObject * ObjectWrapper_getattr(ObjectWrapper *self, PyObject *name) {
    PyObject *value = PyObject_GenericGetAttr((PyObject *)self, name);

    if(value != NULL)
        return value;

    char *_name = PyString_AsString(name);

    HandleScope handle_scope;

    Context::Scope context_scope(self->context);

    Handle<Value> result = self->object->GetRealNamedProperty(String::New(_name));

    if(result.IsEmpty()) {
        // Exception already set by GetAttr
        return NULL;
    }

    // Have to reset exception raised by GetAttr
    PyErr_Clear();

    return wrap(self->context, self->object, result);
};

extern "C" int ObjectWrapper_setattr(ObjectWrapper *self, PyObject *name, PyObject *value) {
    if(PyObject_GenericSetAttr((PyObject *)self, name, value) == 0)
        return 0;

    PyErr_Clear();

    HandleScope handle_scope;

    Context::Scope context_scope(self->context);

    TryCatch trycatch;

    bool ok;

    if(value == NULL) {
        ok = self->object->Delete(String::New(PyString_AsString(name)));
    } else {
        try {
            ok = self->object->Set(String::New(PyString_AsString(name)), unwrap(value));
        } catch (UnwrapError error) {
            error.set_exception();

            return -1;
        }
    }

    if(!ok) {
        set_exception(trycatch);

        return -1;
    }

    return 0;
};

extern "C" PyObject * FunctionWrapper_call(ObjectWrapper *self, PyObject *args, PyObject *kw) {
    int argc = (int)PySequence_Size(args);

    HandleScope handle_scope;

    Context::Scope context_scope(self->context);

    Handle<Value> argv[argc];

    try {
        for(int i = 0; i < argc; i++) {
            argv[i] = unwrap(PySequence_GetItem(args, i));
        }
    } catch (UnwrapError error) {
        error.set_exception();

        return NULL;
    }

    TryCatch trycatch;

    Handle<Object> recv = self->parent;

    if(recv.IsEmpty())
        recv = self->context->Global();

    Handle<Value> result = self->object.As<Function>()->Call(recv, argc, argv);

    if(result.IsEmpty()) {
        set_exception(trycatch);

        return NULL;
    }

    return wrap(self->context, Handle<Object>(), result);
};

void set_exception(TryCatch &trycatch) {
    Handle<Value> exception = trycatch.Exception();

    String::AsciiValue name(exception.As<Object>()->GetConstructorName());

    for(supported_error_type *t = supported_errors; t->name; t++) {
        if(strcmp(*name, t->name) == 0) {
            Handle<Message> message = trycatch.Message();

            PyObject *msg = pythonify_primitive(exception.As<Object>()->Get(String::New("message")));

            Handle<Value> filename = message->GetScriptResourceName();

            if(filename->IsUndefined()) {
                filename = String::New("<string>");
            }

            PyObject *exc_value;

            int lineno = message->GetLineNumber();

            if(lineno) {
                int offset = message->GetEndColumn();

                PyObject *py_filename = pythonify_primitive(filename);

                PyObject *text = pythonify_primitive(message->GetSourceLine());

                exc_value = Py_BuildValue("N(NiiN)", msg, py_filename, lineno, offset, text);
            } else {
                exc_value = msg;
            }

            PyErr_SetObject(t->type, exc_value);

            return;
        }
    }

    PyErr_SetObject(PyExc_Exception, wrap(Context::GetCurrent(), Handle<Object>(), exception));
}

Handle<Value> unwrap(PyObject *py) {
    if(py == Py_True)
        return True();

    if(py == Py_False)
        return False();

    if(py == Py_None)
        return Null();

    if(PyInt_Check(py)) {
        long value = PyInt_AS_LONG(py);

        if(sizeof(long) == sizeof(int32_t)) {
            return Integer::New(value);
        } else {
            if(-value < 1ll << 16 && value < 1ll << 16)
                return Integer::New(value);

            if(value > 0ll && value < 1ll << 32)
                return Integer::NewFromUnsigned(value);
        }
    }

    if(PyFloat_Check(py))
        return Number::New(PyFloat_AS_DOUBLE(py));

    if(PyString_Check(py)) {
        PyObject *unicode = PyUnicode_FromObject(py);
        PyObject *py_string = PyUnicode_AsUTF8String(unicode);

        Handle<String> js_string = String::New(PyString_AS_STRING(py_string));

        Py_DECREF(py_string);
        Py_DECREF(unicode);

        return js_string;
    }

    if(PyUnicode_Check(py)) {
        PyObject *py_string = PyUnicode_AsUTF8String(py);

        Handle<String> js_string = String::New(PyString_AS_STRING(py_string));

        Py_DECREF(py_string);

        return js_string;
    }

    if(PyObject_IsInstance(py, (PyObject *)&ObjectWrapperType)) {
        ObjectWrapper *wrapper = (ObjectWrapper *)py;

        return wrapper->object;
    }

    if(PyList_Check(py)) {
        uint32_t length = PyList_GET_SIZE(py);

        Handle<Array> array = Array::New(length);

        for(uint32_t i = 0; i < length; i++) {
            Handle<Value> item = unwrap(PyList_GET_ITEM(py, i));

            array->Set(i, item);
        }

        return array;
    }

    if(PyDict_Check(py)) {
        PyObject *key, *value;
        Py_ssize_t pos = 0;

        Handle<Object> object = Object::New();

        while(PyDict_Next(py, &pos, &key, &value)) {
            Handle<Value> js_key = unwrap(key);

            object->Set(js_key, unwrap(value));
        }

        return object;
    }

    throw UnwrapError(py);
};

UnwrapError::UnwrapError(PyObject *object) {
    m_object = object;
}

void UnwrapError::set_exception() {
    PyObject *message = get_message();

    PyErr_SetObject(PyExc_TypeError, message);

    Py_DECREF(message);
}

PyObject * UnwrapError::get_message() {
    PyObject *repr = PyObject_Repr(m_object);

    if(repr == NULL) {
        repr = PyString_FromString("<unpresentable object>");
    }

    PyObject *error = PyString_FromFormat("cannot represent %s to javascript", PyString_AS_STRING(repr));

    Py_DECREF(repr);

    return error;
}

PyObject * wrap(Handle<Context> context, Handle<Object> parent, Handle<Value> value) {
    ObjectWrapper *object;

    object = (ObjectWrapper *)pythonify_primitive(value);

    if(object != NULL)
        return (PyObject *)object;

    if (value->IsFunction()) {
        object = PyObject_New(ObjectWrapper, &FunctionWrapperType);
    } else {
        object = PyObject_New(ObjectWrapper, &ObjectWrapperType);
    }

    object->context = Persistent<Context>::New(context);
    object->parent = Persistent<Object>::New(parent);
    object->object = Persistent<Object>::New(value.As<Object>());

    return (PyObject *)object;
}

PyObject * pythonify_primitive(Handle<Value> value) {
    if(value->IsInt32())
        return PyInt_FromLong(value->Int32Value());

    if(value->IsNumber())
        return PyFloat_FromDouble(value->NumberValue());

    if(value->IsBoolean())
        return PyBool_FromLong(value->BooleanValue());

    if(value->IsNull())
        Py_RETURN_NONE;

    if(value->IsUndefined()) {
        return Py_INCREF(PyUndefined), PyUndefined;
    }

    if(value->IsString()) {
        String::Utf8Value string(value);
        return PyUnicode_DecodeUTF8(*string, string.length(), NULL);
    }

    return NULL;
}

PyObject * pythonify(Handle<Value> value) {
    uint32_t i;
    PyObject *py_value;

    PyObject *object = pythonify_primitive(value);

    if(object)
        return object;

    if(value->IsArray()) {
        Handle<Array> array = value.As<Array>();
        uint32_t array_length = array->Length();

        object = PyList_New(array_length);

        if(object == NULL)
            return NULL;

        for(i = 0; i < array_length; i++) {
            py_value = pythonify(array->Get(i));

            if(py_value == NULL) {
                Py_DECREF(object);

                return NULL;
            }

            PyList_SET_ITEM(object, i, py_value);
        }

        return object;
    } else {
        object = PyDict_New();

        if(object == NULL)
            return NULL;

        Handle<Object> js_object = value.As<Object>();

        Handle<Array> properties = js_object->GetOwnPropertyNames();

        uint32_t properties_length = properties->Length();

        for(i = 0; i < properties_length; i++) {
            Handle<Value> name = properties->Get(i);
            Handle<Value> value = js_object->Get(name);

            PyObject *py_key = pythonify(name);

            if(py_key == NULL) {
                Py_DECREF(object);
                return NULL;
            }

            py_value = pythonify(value);

            if(py_value == NULL) {
                Py_DECREF(object);
                Py_DECREF(py_key);

                return NULL;
            }

            if(PyDict_SetItem(object, py_key, py_value) == -1) {
                Py_DECREF(object);
                Py_DECREF(py_key);
                Py_DECREF(py_value);

                return NULL;
            }
        }

        return object;
    }
}

extern "C" PyObject * Hemi_pythonify(PyObject *self, PyObject *args) {
    PyObject *object;

    if(!PyArg_ParseTuple(args, "O", &object))
        return NULL;

    if(PyObject_IsInstance(object, (PyObject *)&ObjectWrapperType)) {
        HandleScope handle_scope;

        ObjectWrapper *wrapper = (ObjectWrapper *)object;

        Context::Scope context_scope(wrapper->context);

        object = pythonify(wrapper->object);
    }

    return object;
}

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
inithemi(void)
{
    PyObject* m;

    if (PyType_Ready(&ContextWrapperType) < 0)
        return;

    if (PyType_Ready(&ObjectWrapperType) < 0)
        return;

    FunctionWrapperType.tp_base = &ObjectWrapperType;

    if (PyType_Ready(&FunctionWrapperType) < 0)
        return;

    if (PyType_Ready(&UndefinedType) < 0)
        return;

    PyUndefined = PyObject_New(PyObject, &UndefinedType);

    m = Py_InitModule3("hemi", module_methods,
                       "Lightweight V8 wrapper.");

    Py_INCREF(&ContextWrapperType);
    PyModule_AddObject(m, "Context", (PyObject *)&ContextWrapperType);

    Py_INCREF(PyUndefined);
    PyModule_AddObject(m, "undefined", PyUndefined);

    for(supported_error_type *t = supported_errors; t->name; t++) {
        Py_INCREF(t->type);
        PyModule_AddObject(m, t->name, t->type);
    }
}
