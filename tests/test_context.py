from nose.tools import raises

import hemi

def test_context():
    c = hemi.Context()

    c.eval(u's = 12')

    assert c.locals.s == 12


@raises(Exception)
def test_error():
    c = hemi.Context()

    c.eval(u'=')
