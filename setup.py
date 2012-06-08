from setuptools import setup, Extension

setup(
    name="hemi", version="0.2.2",
    ext_modules=[
        Extension(
            "hemi",
            sources=["hemi.cpp"],
            libraries=["v8"],
        )
    ],
    classifiers=[
        "Development Status :: 3 - Alpha",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: C++",
    ]
)
