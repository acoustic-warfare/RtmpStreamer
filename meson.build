project(
  'Dynamic Rtmp Streamer',
  'cpp',
  'cython',
  version: '0.1.0',
  meson_version: '>= 1.1.0',
  default_options: ['buildtype=release', 'cpp_std=c++17'],
)

include_dirs = [include_directories('include')]

# ----------------------------------------- #
# source files
# ----------------------------------------- #
cpp_files = files('src/rtmp.cpp')

# ----------------------------------------- #
# Dependencies
# ----------------------------------------- #
gstreamer_dep = dependency('gstreamer-1.0', required: true)
opencv_dep = dependency('opencv4', version: '>= 4.0', required: true)
thread_dep = dependency('threads', required: true)
gst_app_dep = dependency('gstreamer-app-1.0', required: true)
fmt_dep = dependency('fmt', required: true)

# ----------------------------------------- #
# library object
# ----------------------------------------- #
#
librtmp_streamer = library(
  'rtmp-streamer',
  sources: cpp_files,
  dependencies: [
    gstreamer_dep,
    gst_app_dep,
    opencv_dep,
    thread_dep,
    fmt_dep,
  ],
  include_directories: include_dirs,
  install: true, # Mark the shared library for installation
)

# install headers
install_headers('include/rtmp.hpp', subdir: 'rtmp-streamer')

# Install pkg-config file
pkg_config = import('pkgconfig')
pkg_config.generate(
  name: 'rtmp-streamer',
  description: 'Dynamic Real-Time library',
  filebase: 'rtmp-streamer',
  libraries: librtmp_streamer,
  version: '0.1.0',
  subdirs: 'include',
  requires: ['gstreamer-1.0', 'opencv4'],
)

if get_option('python-bindings')
  subdir('bindings')
endif

if get_option('build-examples')
  subdir('examples')
endif
