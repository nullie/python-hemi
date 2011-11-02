from distutils.core import setup, Extension

setup(
    name="hemi", version="1.0",
    ext_modules=[Extension("hemi", ["hemi.cpp"])]
)
