project('DynRT', 'cpp')

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

library(
  'DynRT',
  sources: src_files,
  dependencies: [gstreamer_dep, opencv_dep],
  include_directories: include_dirs,
)
