import hemi

def test_pythonify():
    source = """
    ({
        "foo": 'bar',
         1: '343',
         'tree': {
             'tree': 3,
             'array': [1, 2, 3],
         }
    })
    """

    ctx = hemi.Context()

    value = hemi.pythonify(ctx.eval(source))

    assert value == {1: u'343', u'foo': u'bar', u'tree': {u'tree': 3, u'array': [1, 2, 3]}}
