#LIBRARIES = opencv_include_dirs, gstreamer_include_dirs)

setup_py = configure_file(
  input: 'setup.py.in',
  output: 'setup.py',
  configuration: {
    'PROJECT_NAME': meson.project_name(),
    'SOURCE_DIR': meson.current_source_dir(),
    'INCLUDE_DIRS': opencv_include_dirs,
    'LIBRARY_DIRS': opencv_library_dirs,
    'LIBRARIES': opencv_libraries,
  },
)
