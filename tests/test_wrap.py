import hemi

def test_reuse():
    source = """
    a = {
       'foo': 'bar',
    }
    """

    c = hemi.Context()

    c.execute(source)

    assert c.globals.a is c.globals.a
