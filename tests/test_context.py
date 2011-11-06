from nose.tools import raises

import hemi

def test_context():
    c = hemi.Context()

    c.execute(u's = 12')

    assert c.globals.s == 12


@raises(Exception)
def test_error():
    c = hemi.Context()

    c.execute(u'=')
