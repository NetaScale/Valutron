flex = find_program('flex', required: true)

lgen = generator(flex,
    output : ['@PLAINNAME@.cc', '@PLAINNAME@.hh'],
    arguments : ['-o', '@OUTPUT0@', '--header-file=@OUTPUT1@', '@INPUT@'])

if get_option('gc') == 'mps'
	subdir('mps/code')
endif

vm = executable('valutronvm', lgen.process('Scanner.l'),
    lemgen.process('Parser.y'),
    'AST.cc', 'Bytecode.cc', 'Main.cc', 'Generation.cc', 'Interpreter.cc',
    'ObjectMemory.cc', 'Objects.cc', 'Scheduling.cc', 'Synth.cc',
    'Primitive.cc', 'Typecheck.cc', 'TypeFlow.cc',
    dependencies: [lemon_headers, mps_dep, meson.get_compiler('c').find_library('ev')])