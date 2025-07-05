project(
  'lox',
  ['c', 'cpp'],
  default_options: [
    'warning_level=3',
    'werror=true',
    'b_vscrt=static_from_buildtype',
  ],
)

lak_subprj = subproject('lak')
lak_dep = lak_subprj.get_variable('lak_dep')

c_cpp_args = lak_subprj.get_variable('c_cpp_args')
cpp_args = lak_subprj.get_variable('cpp_args')
c_args = lak_subprj.get_variable('c_args')

override_options = lak_subprj.get_variable('override_options')
override_options_w0 = lak_subprj.get_variable('override_options_w0')
override_options_w1 = lak_subprj.get_variable('override_options_w1')
override_options_w2 = lak_subprj.get_variable('override_options_w2')
override_options_w3 = lak_subprj.get_variable('override_options_w3')
override_options_werror = lak_subprj.get_variable('override_options_werror')

add_project_arguments(c_cpp_args + cpp_args, language: ['cpp'])
add_project_arguments(c_cpp_args + c_args, language: ['c'])

subdir('jlox')
subdir('clox')

executable(
  'jlox',
  jlox,
  override_options: override_options_werror,
  include_directories: include_directories([
    'jlox',
    'include',
  ]),
  dependencies: [
    lak_dep,
  ],
)

executable(
  'clox',
  clox,
  override_options: override_options_werror,
  include_directories: include_directories([
    'clox',
    'include',
  ]),
  dependencies: [
    lak_dep,
  ],
)
