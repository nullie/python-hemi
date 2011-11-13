python-hemi
===========

Synopsis
--------

Hemi is a way to execute Javascript code from Python. It uses Google's V8.

Example usage
-------------

    >>> import hemi
    >>> ctx = hemi.Context()
    >>> ctx.eval('2 + 3')
    5
    >>> ctx.eval('foo = 34')
    34
    >>> ctx.locals.foo
    34
    >>> ctx.eval('function test() { return this.toString() }')
    <hemi.Undefined object at 0x7fcdab78d260>
    >>> ctx.locals.test()
    u'[object global]'
    >>> ctx.locals.test.apply('hello!')
    u'hello!'

Bugs
----

- None yet known.

Todo
----

- new operator.
