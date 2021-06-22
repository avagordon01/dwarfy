project(
  'elfy',
  'cpp',
  default_options: [
    'prefix=' + join_paths(meson.source_root(), 'packaged'),
    'buildtype=debug',
    'warning_level=2',
    'werror=true',
    'cpp_std=c++20',
  ]
)

add_project_arguments(
  '-Wno-unused-variable',
  '-Wno-unused-function',
  '-Wno-unused-parameter',
  '-Wno-unused-private-field',
  '-fdebug-default-version=4',
  language: 'cpp'
)

expected_dep = declare_dependency(
  include_directories: [
    'vendor/expected/include',
  ]
)

executable(
  'elfy-test',
  [
    'elfy-test.cc',
  ],
  dependencies: [
    expected_dep,
  ],
  install: true,
)
