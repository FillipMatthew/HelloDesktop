project('Hello Desktop', 'c', 'cpp', default_options : ['c_std=c18', 'cpp_std=c++23'])

win = import('windows')

src = ['main.cpp', 'Renderer.cpp']
src += win.compile_resources('HelloDesktop.rc', [])

executable('HelloDesktop', src, install : true, install_dir : meson.current_source_dir() + '/bin')
