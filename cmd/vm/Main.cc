#include <cassert>
#include <ctime>

#include "AST.hh"
#include "Compiler.hh"
#include "ObjectMemory.hh"
#include "Objects.hh"
#include "Typecheck.hh"
#include "Interpreter.hh"
#include "CPUThread.hh"

extern void run(ObjectMemory & omem);

int
main(int argc, char * argv[])
{
	void *marker = &marker;
	ObjectMemory omem(marker);
	ProcessOop firstProcess;
	MethodOop start;
	ClassOop initial;

	printf("Valutron\n");
	printf("GC: " VT_GCNAME "\n");

	Primitive::initialise();
	omem.setupInitialObjects();

	ProgramNode *node = MVST_Parser::parseFile(argv[1]);
	SynthContext sctx(omem);
	node->registerNames(sctx);
	node->synth(sctx);
	node->typeReg(sctx.tyChecker());
	node->typeCheck(sctx.tyChecker());
	node->generate(omem);

	initial = ObjectMemory::objGlobals->symbolLookup(
	    SymbolOopDesc::fromString(omem, "INITIAL")).as<ClassOop>();
	assert(!initial.isNil());

	start = initial->methods->symbolLookup(SymbolOopDesc::fromString(omem,
	    "initial")).as<MethodOop>();
	assert(!start.isNil());

#if 0
	firstProcess = ProcessOopDesc::allocate(omem);
	ContextOop ctx = (void*)&firstProcess->stack->basicAt0(0);
	ctx->initWithMethod(omem, Oop(),start);
	firstProcess->context = ctx;

	clock_t begin = clock();
	while(execute(omem, firstProcess, 10000000));
	clock_t end = clock();
	std::cerr << "Completed in " << (double)(end - begin) / CLOCKS_PER_SEC << "\n";
	std::cerr << "Process returned:\n\t";
	firstProcess->accumulator.print(4);

	run(omem);
#endif

	CPUThreadPair mainThread(omem);

	return 0;
}