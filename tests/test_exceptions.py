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
