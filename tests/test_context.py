from nose.tools import raises

import hemi

def test_context():
    c = hemi.Context(u's = 12')

    assert c.s == 12


@raises(Exception)
def test_error():
    hemi.Context(u'=')
