# ----------------------------------------- #
# Source files
# ----------------------------------------- #
pyx_files = [files('rtmp_streamer.pyx')]

# ----------------------------------------- #
# Dependencies
# ----------------------------------------- #
py = import('python').find_installation()
py_dep = py.dependency()

pyx_dependencies = [opencv_dep, py_dep, gst_app_dep, gstreamer_dep]

# ----------------------------------------- #
# Header files
# ----------------------------------------- #
pyx_include_dirs = include_dirs + run_command(
  py,
  [
    '-c', 'import os; import numpy; print(os.path.abspath(numpy.get_include()))',
  ],
  check: true,
).stdout().strip()

# ----------------------------------------- #
# Options
# ----------------------------------------- #
cython_args = ['-Xboundscheck=False']

# ----------------------------------------- #
# bindings
# ----------------------------------------- #
cython_extension = py.extension_module(
  'rtmp_streamer',
  sources: pyx_files,
  dependencies: pyx_dependencies,
  include_directories: pyx_include_dirs,
  cython_args: cython_args,
  link_with: libdynrt,
  override_options: ['cython_language=cpp'],
  cpp_args: ['-DNPY_NO_DEPRECATED_API=NPY_1_7_API_VERSION'],
  install: true,
)
