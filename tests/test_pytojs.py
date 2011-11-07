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
