from datetime import datetime

from nose.tools import raises

import hemi

def test_pass_back():
    ctx = hemi.Context()

    obj = ctx.eval("({1: 3})")

    func = ctx.eval("(function(o) { return o[1] + 3 })")

    val = func(obj)

    assert val == 6

    func2 = ctx.eval("(function(f) { return f({1: 5}) + 2 })")

    val = func2(func)

    assert val == 10


def test_modification():
    ctx = hemi.Context()

    d = ctx.eval("({1: 2})")

    d.foo = 'bar'

    f = ctx.eval("(function(d, k) { return d[k] })")

    assert f(d, 'foo') == 'bar'

    d['bar'] = 'baz'

    assert f(d, 'bar') == 'baz'

    assert d['bar'] == 'baz'


def test_object():
    ctx = hemi.Context()

    o = ctx.Object()

    o.foo = 'bar'

    assert o.foo == 'bar'


def test_function():
    ctx = hemi.Context()

    def callable(this, *args):
        return 'ok'

    f = ctx.Function(callable)

    rv = f()

    assert rv == 'ok'
