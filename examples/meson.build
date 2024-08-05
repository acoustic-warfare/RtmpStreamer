# ----------------------------------------- #
# C++ example
# ----------------------------------------- #

# Dependencies
dependencies = [dependency('rtmp-streamer'), opencv_dep]

# Executable
# NOTE: this executable will not build unless the rtmp-streamer
#       dependency has been built and installed
executable(
  'example_cpp',
  sources: ['example.cpp'],
  dependencies: dependencies,
  include_directories: include_dirs,
  install: false,
)

# ----------------------------------------- #
# Python example
# ----------------------------------------- #

# Configure data
config_data = configuration_data()

# NOTE: running python file requires that rtmp-streamer library has
#       been built and installed on the system
configure_file(
  input: 'example.py.in',
  output: 'example.py',
  configuration: config_data,
)
