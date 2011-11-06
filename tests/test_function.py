import hemi

def test_call():
    source = """
    function test() {
        return 123;
    }
    """

    context = hemi.Context()

    context.execute(source)

    f = context.globals.test

    assert f(234) == 123


def test_args():
    source = """
    function test(x) {
        return x + 12;
    }
    """

    context = hemi.Context()

    context.execute(source)

    val = context.globals.test(24)

    assert val == 36
