project(
  'DynRT',
  'cpp',
  'cython',
  version: '0.1.0',
  default_options: ['buildtype=release', 'cpp_std=c++17'],
)

# Define directories
include_dirs = include_directories('include', '/usr/include')
src_dir = 'src'
bindings_dir = 'bindings/'
py_bindings_install_dir = join_paths(
  get_option('prefix'),
  'lib',
  'python3.10',
  'site-packages',
)

# Source files
cpp_files = files(join_paths(src_dir, 'rtmp.cpp'))
pyx_files = files(join_paths(bindings_dir, 'rtmp_streamer.pyx'))

# Dependencies
gstreamer_dep = dependency('gstreamer-1.0', version: '>=1.0', required: true)
opencv_dep = dependency('opencv4', required: true)
thread_dep = dependency('threads', required: true)
gst_app_dep = dependency('gstreamer-app-1.0', required: true)

# Build the C++ library
libdynrt = library(
  'DynRT',
  sources: cpp_files,
  dependencies: [gstreamer_dep, opencv_dep, thread_dep],
  include_directories: include_dirs,
  cpp_args: ['-std=c++17'], # Ensure C++17 standard is used
  install: true, # Mark the shared library for installation
  install_dir: get_option('libdir'),
)

# Python module setup
py = import('python').find_installation()
py_dep = py.dependency()

# Cython compiler options
cython_args = ['-Xboundscheck=False']

cython_extension = py.extension_module(
  'rtmp_streamer',
  sources: pyx_files,
  dependencies: [opencv_dep, py_dep, gst_app_dep, gstreamer_dep],
  include_directories: include_dirs,
  cython_args: cython_args,
  link_with: libdynrt,
  override_options: ['cython_language=cpp'], # Ensure to compile cython to c++
  cpp_args: ['-std=c++17'], # Ensure C++17 standard is used
  install: true,
  install_dir: py_bindings_install_dir,
)

# install headers
install_headers('include/rtmp.hpp', subdir: 'DynRT')

# Install pkg-config file
pkg_config = import('pkgconfig')
pkg_config.generate(
  name: 'DynRT',
  description: 'Dynamic Real-Time library',
  filebase: 'dynrt',
  libraries: libdynrt,
  version: '0.1.0',
  subdirs: 'include',
  requires: ['gstreamer-1.0', 'opencv4'],
)
