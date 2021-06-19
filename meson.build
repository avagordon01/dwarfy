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
  '-fdebug-default-version=4',
  language: 'cpp'
)

executable(
  'elfy-test',
  [
    'elfy-test.cc',
  ],
  install: true,
)
