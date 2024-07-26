project(
  'DynRT',
  'cpp',
  version: '0.1.0',
  default_options: ['buildtype=release', 'cpp_std=c++17'],
)

gstreamer_dep = dependency(
  'gstreamer-1.0',
  version: '>=1.0',
  required: true,
)
opencv_dep = dependency(
  'opencv4',
  required: true,
)

src_files = ['src/rtmp.cpp']
include_dirs = include_directories(['include/'])

libdynrt = shared_library(
  'DynRT',
  sources: src_files,
  dependencies: [gstreamer_dep, opencv_dep],
  include_directories: include_dirs,
  install: true, # Mark the shared library for installation
)

# Optionally install headers
install_headers('include/rtmp.hpp', subdir: 'DynRT')

# Optionally install pkg-config file
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
