# ----------------------------------------- #
# C++ example
# ----------------------------------------- #

# Dependencies
dependencies = [dependency('dynrt'), opencv_dep]

# Executable
test_exe = executable(
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

configure_file(
  input: 'example.py.in',
  output: 'example.py',
  configuration: config_data,
)
