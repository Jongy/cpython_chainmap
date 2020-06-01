from distutils.core import setup, Extension

chainam_mod = Extension('chainmap',
                        sources=['chainmap.c'])

setup(name='chainmap',
      version='1.0',
      description='TODO',
      ext_modules=[chainam_mod])
