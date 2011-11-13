import sys, traceback

from datetime import datetime

from nose.tools import raises

import hemi


@raises(TypeError)
def test_exception1():
    source = """
    function f(a) {
        return a + 1;
    }
    """

    c = hemi.Context()

    c.eval(source)

    c.locals.f(datetime.now())


def test_exception2():
    ctx = hemi.Context()

    try:
        ctx.eval('  \n = ');
    except hemi.SyntaxError, e:
        assert e.msg == "Unexpected token ="
        assert e.filename == "<string>"
        assert e.lineno == 2
        assert e.offset == 2
        assert e.text == " = "


def test_string_exception():
    ctx = hemi.Context()

    try:
        ctx.eval("throw 'hello'")

        assert False
    except Exception, e:
        assert e.message == "hello"


def test_object_exception():
    ctx = hemi.Context()

    try:
        ctx.eval("throw {foo : 'bar'}")

        assert False
    except Exception, e:
        assert e.message.foo == 'bar'


def test_function_call():
    ctx = hemi.Context()

    source = """
    function f() {
        throw('test')
    }
    """

    ctx.eval(source)

    try:
        ctx.locals.f()

        assert False
    except Exception, e:
        assert e.message == 'test'



def test_python_exception():
    def callable(this, *args):
        raise Exception("test")

    ctx = hemi.Context()

    f = ctx.Function(callable)

    try:
        f()
        assert False
    except hemi.Error, e:
        assert e.message == 'Exception: test'


def test_traceback():
    source = """
    function f1() {
        f.f;
    }
    function f2() {
        f1();
    }
    function f3() {
        f2();
    }
    """
    ctx = hemi.Context()

    ctx.eval(source)

    try:
        ctx.locals.f3()

        assert False
    except Exception, e:
        etype, value, tb = sys.exc_info()

        lines = traceback.format_exception(etype, value, tb)

        assert lines[2:] == [
            u'  File "<string>", line 9, in f3\n',
            u'  File "<string>", line 6, in f2\n',
            u'  File "<string>", line 3, in f1\n',
            u'  File "<string>", line 3\n',
            u'    f.f;\n',
            u'    ^\n', 'ReferenceError: f is not defined\n',
        ]
