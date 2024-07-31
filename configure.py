#!/bin/env python3
import sys
import platform

info={}

arguments = {
'python_env': sys.prefix,
}

contents = '''[binaries]
c = 'gcc'
cpp = 'g++'
cython = '{python_env}/bin/cython'
python = '{python_env}/bin/python'

[built-in options]
prefix = '{python_env}'
libdir = '{python_env}/lib'
'''.format(**arguments)


with open('native-file.ini', 'w') as native_file:
    native_file.write(contents)

