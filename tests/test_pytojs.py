from datetime import datetime

from nose.tools import raises

import hemi

@raises(TypeError)
def test_exception():
    source = """
    function f(a) {
        return a + 1;
    }
    """

    c = hemi.Context()

    c.eval(source)

    c.locals.f(datetime.now())

def test_pass_back():
    ctx = hemi.Context()

    obj = ctx.eval("({1: 3})")

    func = ctx.eval("(function(o) { return o[1] + 3 })")

    val = func(obj)

    assert val == 6

    func2 = ctx.eval("(function(f) { return f({1: 5}) + 2 })")

    val = func2(func)

    assert val == 10
