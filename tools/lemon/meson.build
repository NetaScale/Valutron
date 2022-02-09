lemon = executable('lemonxx', 'lemon.c', c_args : '-DLEMONPLUSPLUS=1', native: true)
lemon_headers = declare_dependency(include_directories: '.')

lemon_tpl = join_paths(meson.current_source_dir(), 'lemxx.tpl')

lemgen = generator(lemon,
	output: ['@BASENAME@.tab.cc', '@BASENAME@.tab.hh'],
	arguments: ['@INPUT@', '-d@BUILD_DIR@', '-p', '-T' + lemon_tpl])