#include <cmath>
#include <csetjmp>

#include "Interpreter.hh"
#include "ObjectMemory.hh"

Oop
unsupportedPrim(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	return (Oop::nil());
}

/*
Prints the number of available object table entries.
Always fails.
Called from Scheduler>>initialize
*/
Oop
primAvailCount(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	// fprintf (stderr, "free: %d\n", availCount ());
	return (Oop::nil());
}

/*
Returns a pseudo-random integer.
Called from
  Random>>next
  Random>>randInteger:
*/
Oop
primRandom(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	short i;
	/* this is hacked because of the representation */
	/* of integers as shorts */
	i = rand() >> 8; /* strip off lower bits */
	if (i < 0)
		i = -i;
	return (SmiOop(i >> 1));
}

extern bool watching;

/*
Inverts the state of a switch.  The switch controls, in part, whether or
not "watchWith:" messages are sent to Methods during execution.
Returns the Boolean representation of the switch value after the invert.
Called from Smalltalk>>watch
*/
Oop
primFlipWatching(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	/* fixme */
	bool watching = !watching;
	return (
	    (Oop)(watching ? ObjectMemory::objTrue : ObjectMemory::objFalse));
}

/*
Changes the current context to argsAt: 1, pushing argsAt: 2 to the top of the
stack.
*/
Oop
primReturnInto(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	/* FIXME: this is ugly */
	ContextOop ctx = args->basicAt(1).as<ContextOop>();
	Oop rVal = args->basicAt(2);
	proc->setContext(ctx);
	printf("Activating return continuation:\n");
	printf("-----------INTO %s>>%s-------------\n",
	    ctx->receiver().isa()->name()->asCStr(),
	    ctx->isBlockContext() ?
		      "<block>" :
		      ctx->methodOrBlock().as<MethodOop>()->selector()->asCStr());
	return rVal;
}

/*
Terminates the interpreter.
Never returns.
Not called from the image.
*/
Oop
primExit(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	exit(0);
}

/*
Returns the class of which the receiver is an instance.
Called from Object>>class
*/
Oop
primClass(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	return (args->basicAt(1).isa());
}

/*
Returns the field count of the von Neumann space of the receiver.
Called from Object>>basicSize
*/
Oop
primSize(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	int i;
	if (args->basicAt(1).isSmi())
		i = 0;
	else
		i = args->basicAt(1).as<MemOop>()->size();
	return (SmiOop(i));
}

/*
Returns a hashed representation of the receiver.
Called from Object>>hash
*/
Oop
primHash(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (args->basicAt(1).isSmi())
		return (args->basicAt(1));
	else
		return (SmiOop(args->basicAt(1).hashCode()));
}

/*
Changes the active process stack if appropriate.  The change causes
control to be returned (eventually) to the context which sent the
message which created the context which invoked this primitive.
Returns true if the change was made; false if not.
Called from Context>>blockReturn
N.B.:  This involves some tricky code.  The compiler generates the
message which invokes Context>>blockReturn.  Context>>blockReturn is a
normal method.  It processes the true/false indicator.  Its result is
discarded when it returns, exposing the value to be returned from the
context which invokes this primitive.  Only then is the process stack
change effective.
*/
Oop
primBlockReturn(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	int i;
	int j;

	/* FIXME:
	// first get previous link pointer
	i = smiOf (orefOf (processStack, linkPointer).val);
	// then creating context pointer
	j = smiOf (orefOf (args->basicAt (1)->ptr, 1).val);
	if (ptrNe (orefOf (processStack, j + 1), args->basicAt (1)))
	    return ((Oop)ObjectMemory::objFalse);
	// first change link pointer to that of creator
	orefOfPut (processStack, i, orefOf (processStack, j));
	// then change return point to that of creator
	orefOfPut (processStack, i + 2, orefOf (processStack, j + 2)); */
	return ((Oop)ObjectMemory::objTrue);
}

jmp_buf jb = {};

void
brkfun(int sig)
{
	longjmp(jb, 1);
}

void
brkignore(int sig)
{
}

// bool execute (encPtr aProcess, int maxsteps);

/*
Executes the receiver until its time slice is ended or terminated.
Returns true in the former case; false in the latter.
Called from Process>>execute
*/
Oop
primExecute(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	/*encPtr saveProcessStack;
	int saveLinkPointer;
	int * saveCounterAddress;*/
	Oop returnedObject;
	// first save the values we are about to clobber
	/* FIXME: saveProcessStack = processStack;
	saveLinkPointer = linkPointer;
	saveCounterAddress = counterAddress;
	// trap control-C
	signal (SIGINT, brkfun);
	if (setjmp (jb))
	    returnedObject = (Oop)ObjectMemory::objFalse;
	else if (execute (args->basicAt (1)->ptr, 1 << 12))
	    returnedObject = (Oop)ObjectMemory::objTrue;
	else
	    returnedObject = (Oop)ObjectMemory::objFalse;
	signal (SIGINT, brkignore);
	// then restore previous environment
	processStack = saveProcessStack;
	linkPointer = saveLinkPointer;
	counterAddress = saveCounterAddress;*/
	return (returnedObject);
}

/*
Returns true if the content of the receiver's Oop is equal to that
of the first argument's; false otherwise.
Called from Object>>==
*/
Oop
primIdent(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (args->basicAt(1) == args->basicAt(2))
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Defines the receiver to be an instance of the first argument.
Returns the receiver.
Called from
  BlockNode>>newBlock
  ByteArray>>asString
  ByteArray>>size:
  Class>>new:
*/
Oop
primClassOfPut(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	//fprintf(stderr, "Setting ClassOf %d to %d\n, ", args->basicAt(1),
	//    args->basicAt(2));
	args->basicAt(1).setIsa(args->basicAt(2).as<ClassOop>());
	return (args->basicAt(1));
}

/*
Creates a new String.  The von Neumann space of the new String is that
of the receiver, up to the left-most null, followed by that of the first
argument, up to the left-most null, followed by a null.
Returns the new String.
Called from
  String>>,
  Symbol>>asString
*/
Oop
primStringCat(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	uint8_t *src1 = args->basicAt(1).as<StringOop>()->vns();
	size_t len1 = strlen((char *)src1);
	uint8_t *src2 = args->basicAt(2).as<StringOop>()->vns();
	size_t len2 = strlen((char *)src2);
	StringOop ans = omem.newByteObj<StringOop>(len1 + len2 + 1);
	uint8_t *tgt = ans->vns();
	(void)memcpy(tgt, src1, len1);
	(void)memcpy(tgt + len1, src2, len2);
	ans.setIsa(ObjectMemory::clsString);
	return ((Oop)ans);
}

/*
Returns the Oop of the receiver denoted by the argument.
Called from Object>>basicAt:
*/
Oop
primBasicAt(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	int i;
	if (args->basicAt(1).isSmi()) {
		printf("integer receiver of basicAt:");
		return (Oop::nil());
	}
	/* if (!args->basicAt (1)->kind == OopsRefObj)
	    return (Oop::nil ()); */
	if (!args->basicAt(2).isSmi()) {
		printf("non-integer argument of basicAt:");
		return (Oop::nil());
	}
	i = args->basicAt(2).as<SmiOop>().smi();
	if (i < 1 || i > args->basicAt(1).as<MemOop>()->size()) {
		printf("#basicAt: argument out of bounds (%d)", i);
		return (Oop::nil());
	}

	return args->basicAt(1).as<OopOop>()->basicAt(i);
}

/*
Returns an encoded representation of the byte of the receiver denoted by
the argument.
Called from ByteArray>>basicAt:
*/
Oop
primByteAt(ObjectMemory &omem, ProcessOop proc, ArrayOop args) /*fix*/
{
	int i;
	if (!args->basicAt(2).isSmi())
		perror("non integer index byteAt:");
	i = args->basicAt(1).as<ByteArrayOop>()->basicAt(
	    args->basicAt(2).as<SmiOop>().smi());
	if (i < 0)
		i += 256;
	return (SmiOop(i));
}

/*
Defines the global value of the receiver to be the first argument.
Returns the receiver.
Called from Symbol>>assign:
*/
Oop
primSymbolAssign(ObjectMemory &omem, ProcessOop proc, ArrayOop args) /*fix*/
{
	ObjectMemory::objGlobals->symbolInsert(omem,
	    args->basicAt(1).as<SymbolOop>(), args->basicAt(2));
	return (args->basicAt(1));
}

/*
Defines the Oop of the receiver denoted by the first argument to be
the second argument.
Returns the receiver.
Called from Object>>basicAt:put:
*/
Oop
primbasicAtPut(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	int i;
	if (args->basicAt(1).isSmi())
		return (Oop::nil());
	if (!args->basicAt(2).isSmi())
		return (Oop::nil());
	i = args->basicAt(2).as<SmiOop>().smi();
	if (i < 1 || i > args->basicAt(1).as<MemOop>()->size())
		return (Oop::nil());
	args->basicAt(1).as<OopOop>()->basicAtPut(i, args->basicAt(3));
	return args->basicAt(1);
}

/*
Defines the byte of the receiver denoted by the first argument to be a
decoded representation of the second argument.
Returns the receiver.
Called from ByteArray>>basicAt:put:
*/
Oop
primByteAtPut(ObjectMemory &omem, ProcessOop proc, ArrayOop args) /*fix*/
{
	int i;
	if (!args->basicAt(2).isSmi())
		perror("#byteAt: non integer index");
	if (!args->basicAt(3).isSmi())
		perror("#byteAt: non integer assignee");
	args->basicAt(1).as<ByteArrayOop>()->basicAtPut(
	    args->basicAt(2).as<SmiOop>().smi(),
	    args->basicAt(3).as<SmiOop>().smi());
	return (args->basicAt(1));
}

inline intptr_t
min(intptr_t one, intptr_t two)
{
	return (one <= two ? one : two);
}

/*
Creates a new String.  The von Neumann space of the new String is
usually that of a substring of the receiver, from the byte denoted by
the first argument through the byte denoted by the second argument,
followed by a null.  However, if the denoted substring is partially
outside the space of the receiver, only that portion within the space of
the receiver is used.  Also, if the denoted substring includes a null,
only that portion up to the left-most null is used.  Further, if the
denoted substring is entirely outside the space of the receiver or its
length is less than one, none of it is used.
Returns the new String.
Called from String>>copyFrom:to:
*/
Oop
primCopyFromTo(ObjectMemory &omem, ProcessOop proc, ArrayOop args) /*fix*/
{
	if ((!args->basicAt(2).isSmi() || (!args->basicAt(3).isSmi())))
		perror("#copyFromTo: non integer index");
	{
		uint8_t *src = args->basicAt(1).as<StringOop>()->vns();
		size_t len = strlen((char *)src);
		int pos1 = args->basicAt(2).as<SmiOop>().smi();
		int pos2 = args->basicAt(2).as<SmiOop>().smi();
		int req = pos2 + 1 - pos1;
		size_t act;
		StringOop ans;
		uint8_t *tgt;
		if (pos1 >= 1 && pos1 <= len && req >= 1)
			act = min(req, strlen(((char *)src) + (pos1 - 1)));
		else
			act = 0;
		ans = omem.newByteObj<StringOop>(act + 1);
		tgt = ans->vns();
		(void)memcpy(tgt, src + (pos1 - 1), act);
		ans.setIsa(ObjectMemory::clsString);
		return ((Oop)ans);
	}
}

Oop
primParse(ObjectMemory &omem, ProcessOop proc, ArrayOop args) /*del*/
{
	/*setInstanceVariables (args->basicAt(1)->ptr);
	if (parse (args->basicAt(2)->ptr, (char *)vnsOf
	(args->basicAt(2)->ptr), false))
	{
	    flushCache (orefOf (args->basicAt(2)->ptr, messageInMethod).ptr,
	args->basicAt(1)->ptr); return ((Oop)memMgr.objTrue());
	}
	else
	    return ((Oop)memMgr.objFalse());*/
}

/*
Returns the equivalent of the receiver's value in a floating-point
representation.
Called from Integer>>asFloat
*/
Oop
primAsFloat(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (!args->basicAt(1).isSmi())
		return (Oop::nil());
	return (args->basicAt(1)); // FIXME:(Oop)FloatOop((double)args->basicAt
				   // (1).as<SmiOop> ().smi ()));
}

/*
Defines a counter to be the argument's value.  When this counter is
less than 1, a Process time slice is finished.
Always fails.
Called from
  Scheduler>>critical:
  Scheduler>>yield
*/
Oop
primSetTimeSlice(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	/*FIXME: if (!args->basicAt (1).isSmi ()))
	    return (Oop::nil ());
	*counterAddress = args->basicAt (1).as<SmiOop> ().smi ();*/
	return (Oop::nil());
}

/*
Returns a new object.  The von Neumann space of the new object will be
presumed to contain a number of Oops.  The number is denoted by the
receiver.
Called from
  BlockNode>>newBlock
  Class>>new:
*/
Oop
primAllocOrefObj(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (!args->basicAt(1).isSmi())
		return (Oop::nil());
	return (omem.newOopObj<MemOop>(args->basicAt(1).as<SmiOop>().smi()));
}

/*
Returns a new object.  The von Neumann space of the new object will be
presumed to contain a number of bytes.  The number is denoted by the
receiver.
Called from
  ByteArray>>size:
*/
Oop
primAllocByteObj(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (!args->basicAt(1).isSmi())
		return (Oop::nil());
	return (omem.newByteObj<MemOop>(args->basicAt(1).as<SmiOop>().smi()));
}

/*
Returns the result of adding the argument's value to the receiver's
value.
Called from Integer>>+
Also called for SendBinary bytecodes.
*/
Oop
primAdd(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	long longresult;

	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi()) {
		printf("nonSMI detected\n");
		return (Oop::nil());
	}
	longresult = args->basicAt(1).as<SmiOop>().smi();
	longresult += args->basicAt(2).as<SmiOop>().smi();
	if (1) // FIXME: bounds test SMI 1) // FIXME: boundscheck smi
		return (SmiOop(longresult));
	else
		return (Oop::nil());
}

/*
Returns the result of subtracting the argument's value from the
receiver's value.
Called from Integer>>-
Also called for SendBinary bytecodes.
*/
Oop
primSubtract(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	long longresult;
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	longresult = args->basicAt(1).as<SmiOop>().smi();
	longresult -= args->basicAt(2).as<SmiOop>().smi();
	if (1) // FIXME: smi boundcheck 1) // FIXME: boundscheck smi
		return (SmiOop(longresult));
	else
		return (Oop::nil());
}

/*
Returns true if the receiver's value is less than the argument's
value; false otherwise.
Called from Integer>><
Also called for SendBinary bytecodes.
*/
Oop
primLessThan(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	if (args->basicAt(1).as<SmiOop>().smi() <
	    args->basicAt(2).as<SmiOop>().smi())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns true if the receiver's value is greater than the argument's
value; false otherwise.
Called from Integer>>>
Also called for SendBinary bytecodes.
*/
Oop
primGreaterThan(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	if (args->basicAt(1).as<SmiOop>().smi() >
	    args->basicAt(2).as<SmiOop>().smi())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns true if the receiver's value is less than or equal to the
argument's value; false otherwise.
Called for SendBinary bytecodes.
*/
Oop
primLessOrEqual(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	if (args->basicAt(1).as<SmiOop>().smi() <=
	    args->basicAt(2).as<SmiOop>().smi())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns true if the receiver's value is greater than or equal to the
argument's value; false otherwise.
Called for SendBinary bytecodes.
*/
Oop
primGreaterOrEqual(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	if (args->basicAt(1).as<SmiOop>().smi() >=
	    args->basicAt(2).as<SmiOop>().smi())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns true if the receiver's value is equal to the argument's value;
false otherwise.
Called for SendBinary bytecodes.
*/
Oop
primEqual(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	if (args->basicAt(1).as<SmiOop>().smi() ==
	    args->basicAt(2).as<SmiOop>().smi())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns true if the receiver's value is not equal to the argument's
value; false otherwise.
Called for SendBinary bytecodes.
*/
Oop
primNotEqual(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	if (args->basicAt(1).as<SmiOop>().smi() !=
	    args->basicAt(2).as<SmiOop>().smi())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns the result of multiplying the receiver's value by the
argument's value.
Called from Integer>>*
Also called for SendBinary bytecodes.
*/
Oop
primMultiply(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	long longresult;
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	longresult = args->basicAt(1).as<SmiOop>().smi();
	longresult *= args->basicAt(2).as<SmiOop>().smi();
	if (1) // FIXME: boundscheck 1) // FIXME: boundscheck smi
		return (SmiOop(longresult));
	else
		return (Oop::nil());
}

/*
Returns the quotient of the result of dividing the receiver's value by
the argument's value.
Called from Integer>>quo:
Also called for SendBinary bytecodes.
*/
Oop
primQuotient(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	long longresult;
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	if (args->basicAt(2).as<SmiOop>().smi() == 0)
		return (Oop::nil());
	longresult = args->basicAt(1).as<SmiOop>().smi();
	longresult /= args->basicAt(2).as<SmiOop>().smi();
	if (1) // FIXME: boundscheck 1) // FIXME: boundscheck smi
		return (SmiOop(longresult));
	else
		return (Oop::nil());
}

/*
Returns the remainder of the result of dividing the receiver's value by
the argument's value.
Called for SendBinary bytecodes.
*/
Oop
primRemainder(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	long longresult;

	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi()) {
		printf("#primRem: receiver or arg not integer");
		return (Oop::nil());
	}
	if (args->basicAt(2).as<SmiOop>().smi() == 0) {
		printf("#primRem: division by zero");
		return (Oop::nil());
	}
	longresult = args->basicAt(1).as<SmiOop>().smi();
	longresult %= args->basicAt(2).as<SmiOop>().smi();
	if (1) // FIXME: boundscheck smi
		return (SmiOop(longresult));
	else
		return (Oop::nil());
}

/*
Returns the bit-wise "and" of the receiver's value and the argument's
value.
Called from Integer>>bitAnd:
Also called for SendBinary bytecodes.
*/
Oop
primBitAnd(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	long longresult;
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	longresult = args->basicAt(1).as<SmiOop>().smi();
	longresult &= args->basicAt(2).as<SmiOop>().smi();
	return (SmiOop(longresult));
}

/*
Returns the bit-wise "exclusive or" of the receiver's value and the
argument's value.
Called from Integer>>bitXor:
Also called for SendBinary bytecodes.
*/
Oop
primBitXor(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	long longresult;
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	longresult = args->basicAt(1).as<SmiOop>().smi();
	longresult ^= args->basicAt(2).as<SmiOop>().smi();
	return (SmiOop(longresult));
}

/*
Returns the result of shifting the receiver's value a number of bit
positions denoted by the argument's value.  Positive arguments cause
left shifts.  Negative arguments cause right shifts.  Note that the
result is truncated to the range of embeddable values.
Called from Integer>>bitXor:
*/
Oop
primBitShift(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	long longresult;
	if (!args->basicAt(1).isSmi() || !args->basicAt(2).isSmi())
		return (Oop::nil());
	longresult = args->basicAt(1).as<SmiOop>().smi();
	if (args->basicAt(2).as<SmiOop>().smi() < 0)
		longresult >>= -args->basicAt(2).as<SmiOop>().smi();
	else
		longresult <<= args->basicAt(2).as<SmiOop>().smi();
	return (SmiOop(longresult));
}

/*
Returns the field count of the von Neumann space of the receiver up to
the left-most null.
Called from String>>size
*/
Oop
primStringSize(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	return (
	    SmiOop(strlen((char *)args->basicAt(1).as<StringOop>()->vns())));
}

int strHash(std::string str);

/*
Returns a hashed representation of the von Neumann space of the receiver
up to the left-most null.
Called from
  String>>hash
  Symbol>>stringHash
*/
Oop
primStringHash(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	return (
	    SmiOop(strHash((char *)args->basicAt(1).as<StringOop>()->vns())));
}

/*
Returns a unique object.  Here, "unique" is determined by the
von Neumann space of the receiver up to the left-most null.  A copy will
either be found in or added to the global symbol table.  The returned
object will refer to the copy.
Called from String>>asSymbol
*/
Oop
primAsSymbol(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	return ((Oop)SymbolOopDesc::fromString(omem,
	    (char *)args->basicAt(1).as<StringOop>()->vns()));
}

/*
Passes the von Neumann space of the receiver to the host's "system"
function.  Returns what that function returns.
Called from String>>unixCommand
*/
Oop
primHostCommand(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	return (
	    SmiOop(system((char *)args->basicAt(1).as<StringOop>()->vns())));
}

/*
Returns the equivalent of the receiver's value in a printable character
representation.
Called from Float>>printString
*/
Oop
primAsString(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	char buffer[32];
	(void)sprintf(buffer, "%g",
	    args->basicAt(1).as<FloatOop>()->floatValue());
	return ((Oop)StringOopDesc::fromString(omem, buffer));
}

/*
Returns the natural logarithm of the receiver's value.
Called from Float>>ln
*/
Oop
primNaturalLog(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	return (
	    (Oop)FloatOop(log(args->basicAt(1).as<FloatOop>()->floatValue())));
}

/*
Returns "e" raised to a power denoted by the receiver's value.
Called from Float>>exp
*/
Oop
primERaisedTo(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	return (
	    (Oop)FloatOop(exp(args->basicAt(1).as<FloatOop>()->floatValue())));
}

/*
Returns a new Array containing two integers n and m such that the
receiver's value can be expressed as n * 2**m.
Called from Float>>integerPart
*/
Oop
primIntegerPart(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	double temp;
	int i;
	int j;
	ArrayOop returnedObject = Oop::nil().as<ArrayOop>();
#define ndif 12
	temp = frexp(args->basicAt(1).as<FloatOop>()->floatValue(), &i);
	if ((i >= 0) && (i <= ndif)) {
		temp = ldexp(temp, i);
		i = 0;
	} else {
		i -= ndif;
		temp = ldexp(temp, ndif);
	}
	j = (int)temp;
	returnedObject = ArrayOopDesc::newWithSize(omem, 2);
	returnedObject->basicAtPut(1, SmiOop(j));
	returnedObject->basicAtPut(2, SmiOop(i));
#ifdef trynew
	/* if number is too big it can't be integer anyway */
	if (args->basicAt(1).as<FloatOop>()->floatValue() > 2e9)
		returnedObject = nil;
	else {
		(void)modf(args->basicAt(1).as<FloatOop>()->floatValue(),
		    &temp);
		ltemp = (long)temp;
		if (canEmbed(ltemp))
			returnedObject = encValueOf((int)temp);
		else
			returnedObject = FloatOop(temp);
	}
#endif
	return ((Oop)returnedObject);
}

/*
Returns the result of adding the argument's value to the receiver's
value.
Called from Float>>+
*/
Oop
primFloatAdd(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	double result;
	result = args->basicAt(1).as<FloatOop>()->floatValue();
	result += args->basicAt(2).as<FloatOop>()->floatValue();
	return ((Oop)FloatOop(result));
}

/*
Returns the result of subtracting the argument's value from the
receiver's value.
Called from Float>>-
*/
Oop
primFloatSubtract(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	double result;
	result = args->basicAt(1).as<FloatOop>()->floatValue();
	result -= args->basicAt(2).as<FloatOop>()->floatValue();
	return ((Oop)FloatOop(result));
}

/*
Returns true if the receiver's value is less than the argument's
value; false otherwise.
Called from Float>><
*/
Oop
primFloatLessThan(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (args->basicAt(1).as<FloatOop>()->floatValue() <
	    args->basicAt(2).as<FloatOop>()->floatValue())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns true if the receiver's value is greater than the argument's
value; false otherwise.
Not called from the image.
*/
Oop
primFloatGreaterThan(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (args->basicAt(1).as<FloatOop>()->floatValue() >
	    args->basicAt(2).as<FloatOop>()->floatValue())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns true if the receiver's value is less than or equal to the
argument's value; false otherwise.
Not called from the image.
*/
Oop
primFloatLessOrEqual(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (args->basicAt(1).as<FloatOop>()->floatValue() <=
	    args->basicAt(2).as<FloatOop>()->floatValue())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns true if the receiver's value is greater than or equal to the
argument's value; false otherwise.
Not called from the image.
*/
Oop
primFloatGreaterOrEqual(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (args->basicAt(1).as<FloatOop>()->floatValue() >=
	    args->basicAt(2).as<FloatOop>()->floatValue())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns true if the receiver's value is equal to the argument's value;
false otherwise.
Called from Float>>=
*/
Oop
primFloatEqual(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (args->basicAt(1).as<FloatOop>()->floatValue() ==
	    args->basicAt(2).as<FloatOop>()->floatValue())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns true if the receiver's value is not equal to the argument's
value; false otherwise.
Not called from the image.
*/
Oop
primFloatNotEqual(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	if (args->basicAt(1).as<FloatOop>()->floatValue() !=
	    args->basicAt(2).as<FloatOop>()->floatValue())
		return ((Oop)ObjectMemory::objTrue);
	else
		return ((Oop)ObjectMemory::objFalse);
}

/*
Returns the result of multiplying the receiver's value by the
argument's value.
Called from Float>>*
*/
Oop
primFloatMultiply(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	double result;
	result = args->basicAt(1).as<FloatOop>()->floatValue();
	result *= args->basicAt(2).as<FloatOop>()->floatValue();
	return ((Oop)FloatOop(result));
}

/*
Returns the result of dividing the receiver's value by the argument's
value.
Called from Float>>/
*/
Oop
primFloatDivide(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	double result;
	result = args->basicAt(1).as<FloatOop>()->floatValue();
	result /= args->basicAt(2).as<FloatOop>()->floatValue();
	return ((Oop)FloatOop(result));
}

#define MAXFILES 32

FILE *fp[MAXFILES] = {};

/*
Opens the file denoted by the first argument, if necessary.  Some of the
characteristics of the file and/or the operations permitted on it may be
denoted by the second argument.
Returns non-nil if successful; nil otherwise.
Called from File>>open
*/
Oop
primFileOpen(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	int i = args->basicAt(1).as<SmiOop>().smi();
	char *p = (char *)args->basicAt(2).as<StringOop>()->vns();
	if (!strcmp(p, "stdin"))
		fp[i] = stdin;
	else if (!strcmp(p, "stdout"))
		fp[i] = stdout;
	else if (!strcmp(p, "stderr"))
		fp[i] = stderr;
	else {
		char *q = (char *)args->basicAt(2).as<StringOop>()->vns();
		char *r = strchr(q, 'b');
		ByteOop s;
		if (r == NULL) {
			int t = strlen(q);
			s = omem.newByteObj<ByteOop>(t + 2);
			r = (char *)s->vns();
			memcpy(r, q, t);
			*(r + t) = 'b';
			q = r;
		}
		fp[i] = fopen(p, q);
		/* FIXME: if (r == NULL)
		    isVolatilePut (s, false);*/
	}
	if (fp[i] == NULL)
		return (Oop::nil());
	else
		return (SmiOop(i));
}

/*
Closes the file denoted by the receiver.
Always fails.
Called from File>>close
*/
Oop
primFileClose(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	int i = args->basicAt(1).as<SmiOop>().smi();
	if (fp[i])
		(void)fclose(fp[i]);
	fp[i] = NULL;
	return (Oop::nil());
}

// void coldFileIn (encVal tagRef);

/*
Applies the built-in "fileIn" function to the file denoted by the
receiver.
Always fails.
Not called from the image.
N.B.:  The built-in function uses the built-in compiler.  Both should be
used only in connection with building an initial image.
*/
Oop
primFileIn(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	/*int i = args->basicAt (1).as<SmiOop> ().smi ();
	if (fp[i])
	    coldFileIn (args->basicAt (1)->val);
	return (Oop::nil ());*/
}

/*
Reads the next line of characters from the file denoted by the receiver.
This line usually starts with the character at the current file position
and ends with the left-most newline.  However, if reading from standard
input, the line may be continued by immediately preceding the newline
with a backslash, both of which are deleted.  Creates a new String.  The
von Neumann space of the new String is usually the characters of the
complete line followed by a null.  However, if reading from standard
input, the trailing newline is deleted.  Also, if the line includes a
null, only that portion up to the left-most null is used.
Returns the new String if successful, nil otherwise.
Called from File>>getString
*/
Oop
primGetString(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	int i = args->basicAt(1).as<SmiOop>().smi();
	int j;
	char buffer[4096];
	if (!fp[i])
		return (Oop::nil());
	j = 0;
	buffer[j] = '\0';
	while (1) {
		if (fgets(&buffer[j], 512, fp[i]) == NULL) {
			if (fp[i] == stdin)
				(void)fputc('\n', stdout);
			return (Oop::nil()); /* end of file */
		}
		if (fp[i] == stdin) {
			/* delete the newline */
			j = strlen(buffer);
			if (buffer[j - 1] == '\n')
				buffer[j - 1] = '\0';
		}
		j = strlen(buffer) - 1;
		if (buffer[j] != '\\')
			break;
		/* else we loop again */
	}
	return ((Oop)StringOopDesc::fromString(omem, buffer));
}

/*
Writes the von Neumann space of the argument, up to the left-most null,
to the file denoted by the receiver.
Always fails.
Called from File>>printNoReturn:
*/
Oop
primPrintWithoutNL(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	int i = args->basicAt(1).as<SmiOop>().smi(); // smiOf
						     // (arg[0].val);
	if (!fp[i])
		return (Oop());
	(void)fputs((char *)args->basicAt(2).as<ByteArrayOop>()->vns(), fp[i]);
	(void)fflush(fp[i]);
	return (Oop());
}

/*
Writes the von Neumann space of the argument, up to the left-most null,
to the file denoted by the receiver and appends a newline.
Always fails.
Called from File>>print:
*/
Oop
primPrintWithNL(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	int i = args->basicAt(1).as<SmiOop>().smi();
	if (!fp[i])
		return (Oop());
	(void)fputs((char *)args->basicAt(2).as<ByteArrayOop>()->vns(), fp[i]);
	(void)fputc('\n', fp[i]);
	return (Oop());
}

Oop
primExecBlock(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	ContextOop ctx = ContextOopDesc::newWithBlock(omem,
	    args->basicAt(1).as<BlockOop>());
	for (int i = 2; i <= args.as<MemOop>()->size(); i++) {
		printf("add argument %d/stack basicAt %d\n", i - 1, i - 2);
		ctx->stack()->basicAtPut(i, args->basicAt(i));
	}

	ctx->setPreviousContext(proc->context()->previousContext());
	proc->setContext(ctx);
	// printf ("=> Entering block\n");
	return Oop();
}

Oop
primDumpVariable(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	ContextOop ctx = proc->context();

	printf("Dump variable:\n");

	args->basicAt(1).print(20);
	args->basicAt(1).isa()->print(20);
	printf("          --> %s>>%s\n",
	    ctx->receiver().isa()->name()->asCStr(),
	    ctx->isBlockContext() ?
		      "<block>" :
		      ctx->methodOrBlock().as<MethodOop>()->selector()->asCStr());
	while ((ctx = ctx->previousContext()) != Oop::nil())
		printf("          --> %s>>%s\n",
		    ctx->receiver().isa()->name()->asCStr(),
		    ctx->isBlockContext() ? "<block>" :
						  ctx->methodOrBlock()
						.as<MethodOop>()
						->selector()
						->asCStr());
	return Oop();
}

Oop
primMsg(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	printf("Debug message:\n\t%s\n",
	    args->basicAt(1).as<StringOop>()->asCStr());
	return Oop();
}

Oop
primFatal(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	printf("Fatal error: %s\n", args->basicAt(1).as<StringOop>()->asCStr());
	abort();
	return Oop();
}

Oop
primExecuteNative(ObjectMemory &omem, ProcessOop proc, ArrayOop args)
{
	//    args->basicAt (1)->asNativeCodeOop ()->fun () ((void *)proc.index
	//    ());
	return Oop();
}

PrimitiveMethod *primVec[255] = {
	/*000*/ &unsupportedPrim,
	/*001*/ &unsupportedPrim,
	/*002*/ &primAvailCount,
	/*003*/ &primRandom,
	/*004*/ &primReturnInto,
	/*005*/ &primFlipWatching,
	/*006*/ &unsupportedPrim,
	/*007*/ &unsupportedPrim,
	/*008*/ &unsupportedPrim,
	/*009*/ &primExit,
	/*010*/ &unsupportedPrim,
	/*011*/ &primClass,
	/*012*/ &primSize,
	/*013*/ &primHash,
	/*014*/ &unsupportedPrim,
	/*015*/ &unsupportedPrim,
	/*016*/ &unsupportedPrim,
	/*017*/ &unsupportedPrim,
	/*018*/ &primBlockReturn,
	/*019*/ &primExecute,
	/*020*/ &unsupportedPrim,
	/*021*/ &primIdent,
	/*022*/ &primClassOfPut,
	/*023*/ &unsupportedPrim,
	/*024*/ &primStringCat,
	/*025*/ &primBasicAt,
	/*026*/ &primByteAt,
	/*027*/ &primSymbolAssign,
	/*028*/ &unsupportedPrim,
	/*029*/ &unsupportedPrim,
	/*030*/ &unsupportedPrim,
	/*031*/ &primbasicAtPut,
	/*032*/ &primByteAtPut,
	/*033*/ &primCopyFromTo,
	/*034*/ &unsupportedPrim,
	/*035*/ &unsupportedPrim,
	/*036*/ &unsupportedPrim,
	/*037*/ &unsupportedPrim,
	/*038*/ &unsupportedPrim, //&primFlushCache,
	/*039*/ &primParse,
	/*040*/ &unsupportedPrim,
	/*041*/ &unsupportedPrim,
	/*042*/ &unsupportedPrim,
	/*043*/ &unsupportedPrim,
	/*044*/ &unsupportedPrim, //&primSpecial,
	/*045*/ &unsupportedPrim,
	/*046*/ &unsupportedPrim,
	/*047*/ &unsupportedPrim,
	/*048*/ &unsupportedPrim,
	/*049*/ &unsupportedPrim,
	/*050*/ &unsupportedPrim,
	/*051*/ &primAsFloat,
	/*052*/ &unsupportedPrim,
	/*053*/ &primSetTimeSlice,
	/*054*/ &unsupportedPrim,
	/*055*/ &unsupportedPrim, //&primSetSeed,
	/*056*/ &unsupportedPrim,
	/*057*/ &unsupportedPrim,
	/*058*/ &primAllocOrefObj,
	/*059*/ &primAllocByteObj,
	/*060*/ &primAdd,
	/*061*/ &primSubtract,
	/*062*/ &primLessThan,
	/*063*/ &primGreaterThan,
	/*064*/ &primLessOrEqual,
	/*065*/ &primGreaterOrEqual,
	/*066*/ &primEqual,
	/*067*/ &primNotEqual,
	/*068*/ &primMultiply,
	/*069*/ &primQuotient,
	/*070*/ &primRemainder,
	/*071*/ &primBitAnd,
	/*072*/ &primBitXor,
	/*073*/ &unsupportedPrim,
	/*074*/ &unsupportedPrim,
	/*075*/ &unsupportedPrim,
	/*076*/ &unsupportedPrim,
	/*077*/ &unsupportedPrim,
	/*078*/ &unsupportedPrim,
	/*079*/ &primBitShift,
	/*080*/ &unsupportedPrim,
	/*081*/ &primStringSize,
	/*082*/ &primStringHash,
	/*083*/ &primAsSymbol,
	/*084*/ &unsupportedPrim,
	/*085*/ &unsupportedPrim,
	/*086*/ &unsupportedPrim,
	/*087*/ &unsupportedPrim,
	/*088*/ &primHostCommand,
	/*089*/ &unsupportedPrim,
	/*090*/ &unsupportedPrim,
	/*091*/ &unsupportedPrim,
	/*092*/ &unsupportedPrim,
	/*093*/ &unsupportedPrim,
	/*094*/ &unsupportedPrim,
	/*095*/ &unsupportedPrim,
	/*096*/ &unsupportedPrim,
	/*097*/ &unsupportedPrim,
	/*098*/ &unsupportedPrim,
	/*099*/ &unsupportedPrim,
	/*100*/ &unsupportedPrim,
	/*101*/ &primAsString,
	/*102*/ &primNaturalLog,
	/*103*/ &primERaisedTo,
	/*104*/ &unsupportedPrim,
	/*105*/ &unsupportedPrim,
	/*106*/ &primIntegerPart,
	/*107*/ &unsupportedPrim,
	/*108*/ &unsupportedPrim,
	/*109*/ &unsupportedPrim,
	/*110*/ &primFloatAdd,
	/*111*/ &primFloatSubtract,
	/*112*/ &primFloatLessThan,
	/*113*/ &primFloatGreaterThan,
	/*114*/ &primFloatLessOrEqual,
	/*115*/ &primFloatGreaterOrEqual,
	/*116*/ &primFloatEqual,
	/*117*/ &primFloatNotEqual,
	/*118*/ &primFloatMultiply,
	/*119*/ &primFloatDivide,
	/*120*/ &primFileOpen,
	/*121*/ &primFileClose,
	/*122*/ &unsupportedPrim,
	/*123*/ &primFileIn,
	/*124*/ &unsupportedPrim,
	/*125*/ &primGetString,
	/*126*/ &unsupportedPrim,
	/*127*/ &unsupportedPrim, //&primImageWrite,
	/*128*/ &primPrintWithoutNL,
	/*129*/ &primPrintWithNL,
	/*130*/ &unsupportedPrim,
	/*131*/ &unsupportedPrim,
	/*132*/ &unsupportedPrim,
	/*133*/ &unsupportedPrim,
	/*134*/ &unsupportedPrim,
	/*135*/ &unsupportedPrim,
	/*136*/ &unsupportedPrim,
	/*137*/ &unsupportedPrim,
	/*138*/ &unsupportedPrim,
	/*139*/ &unsupportedPrim,
	/*140*/ &unsupportedPrim,
	/*141*/ &unsupportedPrim,
	/*142*/ &unsupportedPrim,
	/*143*/ &unsupportedPrim,
	/*144*/ &unsupportedPrim,
	/*145*/ &unsupportedPrim,
	/*146*/ &unsupportedPrim,
	/*147*/ &unsupportedPrim,
	/*148*/ &unsupportedPrim,
	/*149*/ &unsupportedPrim,
	/*150*/ &unsupportedPrim,
	/*151*/ &unsupportedPrim, //&primSetTrace,
	/*152*/ &unsupportedPrim, //&primError,
	/*153*/ &unsupportedPrim, //&primReclaim,
	/*154*/ &unsupportedPrim, //&primLogChunk,
	/*155*/ &unsupportedPrim,
	/*156*/ &unsupportedPrim,
	/*157*/ &unsupportedPrim, //&primGetChunk,
	/*158*/ &unsupportedPrim, //&primPutChunk,
	/*159*/ &unsupportedPrim,
	/*160*/ &primExecBlock,
	/*161*/ &primDumpVariable,
	/*162*/ &primMsg,
	/*163*/ &primFatal,
	/*164*/ &primExecuteNative,
};