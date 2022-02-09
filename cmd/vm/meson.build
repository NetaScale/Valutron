flex = find_program('flex', required: true)

lgen = generator(flex,
    output : ['@PLAINNAME@.cc', '@PLAINNAME@.hh'],
    arguments : ['-o', '@OUTPUT0@', '--header-file=@OUTPUT1@', '@INPUT@'])

if get_option('gc') == 'mps'
	subdir('mps/code')
endif

vm = executable('valutronvm', lgen.process('Scanner.l'),
    lemgen.process('Parser.y'),
    dependencies: [lemon_headers, mps_dep])