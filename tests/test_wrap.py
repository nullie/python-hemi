import hemi

def test_reuse():
    source = """
    a = {
       'foo': 'bar',
    }
    """

    c = hemi.Context()

    c.eval(source)

    assert c.locals.a is c.locals.a
