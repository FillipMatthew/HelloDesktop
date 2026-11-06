project('Hello Desktop', 'c', 'cpp', default_options : ['c_std=c23', 'cpp_std=c++23'])

cpp_compiler = meson.get_compiler('cpp')

# Add the required GCC module flags
add_project_arguments('-fmodules', language : 'cpp')

# Locate GCC's std.cc and bootstrap the 'std' module
bootstrap_std = custom_target('bootstrap_std_module',
  output : 'gcm.cache',
  command : [
    cpp_compiler.cmd_array(),
    '-std=c++23',
    '-fmodules',
    '-fsearch-include-path',
    '-c', 'bits/std.cc'
  ],
  build_by_default : true
)

# Create a dependency wrapper to force compilation order
std_module_dep = declare_dependency(sources : bootstrap_std)

win = import('windows')

src = ['src/main.cpp', 'src/HelloDesktop.cpp', 'src/BackgroundWindowManager.cpp', 'src/Renderer.cpp', 'src/TrayIcon.cpp']
src += win.compile_resources('resources/HelloDesktop.rc',
  depend_files : ['resources/HelloDesktop.exe.manifest'],
  include_directories : include_directories('resources')
)

# Pass the dependency wrapper via 'dependencies'
executable('HelloDesktop',
  src,
  dependencies : std_module_dep,
  win_subsystem : 'windows',
  link_args : ['-static'],
  install : true,
  install_dir : meson.current_source_dir() / 'bin' / get_option('buildtype')
)
