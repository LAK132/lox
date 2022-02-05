project(
  'lox',
  ['c', 'cpp'],
  default_options: [
    'warning_level=3',
    'werror=true',
    'b_vscrt=static_from_buildtype',
  ],
)

cxx = meson.get_compiler('cpp')

if cxx.get_id() == 'msvc'
  version = 'c++latest'

  c_cpp_args = [
    '-DUNICODE',
    '-DWIN32_LEAN_AND_MEAN',
    '-DNOMINMAX',
    '-wd4315',
    '-wd4366',
    '-Wv:18',
  ]

  cpp_args = [
    '-Zc:__cplusplus',
    '-Zc:rvalueCast',
    '-Zc:wchar_t',
    '-Zc:ternary',
    '-Zc:preprocessor',
  ]

  c_args = [
    '-W0',
  ]

  macro_test_args = c_cpp_args + cpp_args + ['-std:' + version]
else
  version = 'c++20'

  c_cpp_args = [
    '-Wno-gnu-zero-variadic-macro-arguments',
    '-Wno-language-extension-token',
    '-Wno-dangling-else',
    '-Wno-microsoft-cast',
    '-Wno-pragma-pack',
    '-Wno-deprecated-declarations',
    '-Wno-unused-private-field',
    '-Wfatal-errors',
  ]

  cpp_args = []

  c_args = []

  macro_test_args = c_cpp_args + cpp_args + ['-std=' + version]
endif

c_cpp_args += [
  '-D_CRT_SECURE_NO_WARNINGS',
]

if get_option('buildtype') != 'debug'
  project_args += [
    '-DNDEBUG',
  ]
endif

add_project_arguments(c_cpp_args + cpp_args, language: ['cpp'])
add_project_arguments(c_cpp_args + c_args, language: ['c'])

cplusplus = cxx.get_define('__cplusplus', args: macro_test_args)

if cxx.compute_int(cplusplus) < 202002
  error('C++ version insufficient, expected 202002L got ' + cplusplus)
endif

subdir('include')
subdir('jlox')
subdir('clox')

executable(
  'jlox',
  jlox,
  override_options: 'cpp_std=' + version,
  include_directories: include_directories([
    'jlox',
    'include',
    'include/lak/inc',
  ]),
  link_with: [
    lak,
  ],
)

executable(
  'clox',
  clox,
  override_options: 'cpp_std=' + version,
  include_directories: include_directories([
    'clox',
    'include',
    'include/lak/inc',
  ]),
  link_with: [
    lak,
  ],
)
