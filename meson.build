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
  '-Wno-exceptions',
  '-fdebug-default-version=4',
  language: 'cpp'
)

dwarfy_lib = static_library('dwarfy',
  'src/elf.cc',
  'src/dwarf.cc',
  'src/enums.cc',
  'src/compilation-unit.cc',
  'src/debugging-information-entry.cc',
  include_directories: [
    'include',
  ],
  dependencies: [
    dependency('range-v3'),
  ],
  install: true,
)
dwarfy_dep = declare_dependency(
  include_directories: [
    'include',
  ],
  link_with: [
    dwarfy_lib
  ],
)

executable(
  'elfy-test',
  [
    'elfy-test.cc',
  ],
  dependencies: [
    dwarfy_dep,
  ],
  install: true,
)
