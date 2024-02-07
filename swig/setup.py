try:
    from setuptools import setup, Extension
except ImportError:
    from distutils.core import setup, Extension

module1 = Extension(
    '_bee2',
    include_dirs=['/usr/local/include'],
    libraries=['bee2'],
    library_dirs=['/usr/local/lib'],
    sources=[
        'bee2py/bee2_wrap.c'
    ],
)


setup(
    name="bee2py",
    version="0.0.1",
    author="Mikhail Mitskevich",
    author_email="mitskevichmn@gmail.com",
    description="Extensions for Bee2 library",
    classifiers=[
        "Programming Language :: C",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
    packages=['bee2py'],
    ext_modules=[module1]
)