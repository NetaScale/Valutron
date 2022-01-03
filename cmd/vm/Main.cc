#include "AST.hh"
#include "Compiler.hh"
#include "ObjectMemory.hh"
#include "Typecheck.hh"

int
main(int argc, char * argv[])
{
	void *marker = &marker;
	ObjectMemory omem(marker);

	omem.setupInitialObjects();

	ProgramNode *node = MVST_Parser::parseFile(argv[1]);
	SynthContext sctx(omem);
	node->registerNames(sctx);
	node->synth(sctx);
	node->typeReg(sctx.tyChecker());
	node->typeCheck(sctx.tyChecker());
	//node->generate(omem);
	//omem.objGlobals->print(5);

	return 0;
}