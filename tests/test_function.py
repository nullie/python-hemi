import hemi

def test_call():
    source = """
    function test() {
        return 123;
    }
    """

    context = hemi.Context()

    context.eval(source)

    f = context.locals.test

    assert f(234) == 123


def test_args():
    source = """
    function test(x) {
        return x + 12;
    }
    """

    context = hemi.Context()

    context.eval(source)

    val = context.locals.test(24)

    assert val == 36


def test_apply():
    source = """
    (function() {
        return this + 2;
    })
    """

    ctx = hemi.Context()

    f = ctx.eval(source)

    v = f.apply(3)

    assert v == 5
