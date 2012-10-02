from setuptools import setup, Extension

setup(
    name="hemi", version="0.2.5",
    ext_modules=[
        Extension(
            "hemi",
            sources=["hemi.cpp"],
            libraries=["v8"],
        )
    ],
    classifiers=[
        "Development Status :: 4 - Beta",
        "License :: OSI Approved :: BSD License",
        "Programming Language :: C++",
    ]
)
