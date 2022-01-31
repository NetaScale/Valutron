/**
 * Defines the Smalltalk object types the VM must be able to work with.
 */

#ifndef OBJECTS_HH_
#define OBJECTS_HH_

#include "Oops.hh"

#define AccessorPair(Type, GetName, SetName, Index)                    \
	inline Type &GetName() { return m_oops[Index].as<Type>(); }     \
	inline Type SetName(Type toValue)                              \
	{                                                              \
		return (m_oops[Index] = toValue.as<Oop>()).as<Type>(); \
	}

class ArrayOopDesc : public OopOopDesc {
    public:
	static ArrayOop newWithSize(ObjectMemory &omem, size_t size);
	static ArrayOop fromVector(ObjectMemory &omem, std::vector<Oop> vec);
	static ArrayOop symbolArrayFromStringVector(ObjectMemory &omem,
	    std::vector<std::string> vec);

	void print(int in);
};

class CacheOopDesc : public OopOopDesc {
	public:
	static const int clsInstLength = 5;

	SymbolOop selector;
	ClassOop cls;
	Smi version;
	MethodOop method;
	CacheOop next;

	static CacheOop newWithSelector(ObjectMemory &omem, SymbolOop value);
};

class CharOopDesc : public OopOopDesc {
    public:
	AccessorPair(Smi, value, setValue, 0);

	static CharOop newWith(ObjectMemory &omem, intptr_t value);
};

class AssociationLinkOopDesc : public OopOopDesc {
    public:
	AccessorPair(Oop, one, setOne, 0);
	AccessorPair(Oop, two, setTwo, 1);
	AccessorPair(AssociationLinkOop, nextLink, setNextLink, 2);

	void print(int in);

	static AssociationLinkOop newWith(ObjectMemory &omem, Oop a, Oop b);
};

class DictionaryOopDesc : public OopOopDesc {
    public:
	/**
	 * Inserts /a value under /a key under the hash /a hash.
	 */
	void insert(ObjectMemory &omem, intptr_t hash, Oop key, Oop value);

	/**
	 * Looks for a key-value pair by searching for the given hash, then
	 * running /a fun on each element that matches, with the value attached
	 * to the key as the first parameter and /a extraVal as the second
	 * parameter. When /a fun returns true, that key-value pair is returned.
	 */
	template <typename ExtraType>
	std::pair<Oop, Oop> findPairByFun(uint32_t hash, ExtraType extraVal,
	    int (*fun)(Oop, ExtraType));

#pragma mark symbol table functions
	/*
	 * Symbol table functions - to be moved later to a dedicated subclass!!!
	 */

	/*
	 * Inserts a value to be associated with a symbol key.
	 */
	void symbolInsert(ObjectMemory &omem, SymbolOop key, Oop value);

	Oop symbolLookup(SymbolOop aSymbol);

	/*
	 * Looks up the value associated with whichever symbol carries string
	 * value aString.
	 */
	Oop symbolLookup(ObjectMemory &omem, std::string aString);

	/*
	 * Namespace functions.
	 */
	ClassOop findOrCreateClass(ObjectMemory &omem, ClassOop superClass,
	    std::string name);
	/**
	 * Create or find a subnamespace with the given name. Name may be
	 * colonised.
	 */
	DictionaryOop subNamespaceNamed(ObjectMemory &omem, std::string name);
	/* Look up a symbol, searching superspaces if necessary. */
	Oop symbolLookupNamespaced(ObjectMemory &omem, std::string aString);

#pragma mark creation

	static DictionaryOop newWithSize(ObjectMemory &omem, size_t numBuckets);

#pragma mark misc
	void print(int in);
};

class ClassOopDesc : public OopOopDesc {
    public:
	/*
	 * For a system class (i.e. one that can't be altered by the running
	 * Smalltalk image), defines the size of one of its instances.
	 */
	static const int clsInstLength = 7;

	SymbolOop	name;
	ClassOop	superClass;
	DictionaryOop	methods;
	Smi		nstSize;
	ArrayOop	nstVars;
	DictionaryOop	dictionary;
	Klass 		*klass;

	const char *nameCStr();

	/**
	 * Adds an instance method to the class. Creates the method dictionary
	 * if necessary. Sets the method's class pointer accordingly.
	 */
	void addMethod(ObjectMemory &omem, MethodOop method);

	/**
	 * Adds a class method to the class. Creates the metaclass' method
	 * dictionary if necessary. Sets the method's class pointer accordingly.
	 */
	void addClassMethod(ObjectMemory &omem, MethodOop method);

	/**
	 * For a class that was raw-allocated, );
	 * allocate() does. Requires that the class has an allocated metaclass
	 * stored into its isa field.
	 */
	void setupClass(ObjectMemory &omem, ClassOop superClass,
	    std::string name);

	/**
	 * Sets the superclass and metaclass' superclass for a given superclass.
	 */
	void setupSuperclass(ClassOop superClass);

	/**
	 * Allocates a raw single class. Does NOT set up its name, superclass,
	 * methods dictionary, size, or instance variables fields.
	 */
	static ClassOop allocateRawClass(ObjectMemory &omem);

	/**
	 * Allocates an empty classpair with the given superclass and name.
	 */
	static ClassOop allocate(ObjectMemory &omem, ClassOop superClass,
	    std::string name);

	void print(int in);
};

class ClassKlass : public Klass {
    public:
	static ClassKlass klass;

	ClassKlass() : Klass(kClass) {};
};

/* Only at:ifAbsent: and at:put: need be implemented to do ByteArrays. The other
 * logic can remain identical. */
class ByteArrayOopDesc : public ByteOopDesc {
    public:
	static ByteArrayOop newWithSize(ObjectMemory &omem, size_t size);
	static ByteArrayOop fromVector(ObjectMemory &omem,
	    std::vector<uint8_t> vec);
};

class StringOopDesc : public ByteArrayOopDesc {
    public:
	static StringOop fromString(ObjectMemory & omem, std::string aString);

	inline bool strEquals(std::string aString)
	{
		return !strcmp((char *)vns(), aString.c_str());
	}
	inline const char *asCStr() { return (const char *)vns(); }
	inline std::string asString() { return (const char *)vns(); }
};

class SymbolOopDesc : public StringOopDesc {
    public:
	static SymbolOop fromString(ObjectMemory &omem, std::string aString);

	void print(int in);
};

class ClassPair {
    public:
	ClassOop cls;
	ClassOop metaCls;

	ClassPair(ClassOop cls, ClassOop metaCls)
	    : cls(cls)
	    , metaCls(metaCls)
	{
	}

	/**
	 * Allocates a raw classpair: no name, no iVar vector, no method
	 * dictionary.
	 */
	static ClassPair allocateRaw(ObjectMemory &omem);
};

class MethodOopDesc : public OopOopDesc {
	static const int clsNstLength = 10;

    public:
	AccessorPair(ByteArrayOop, bytecode, setBytecode, 0);
	AccessorPair(ArrayOop, literals, setLiterals, 1);
	AccessorPair(Smi, argumentCount, setArgumentCount, 2);
	AccessorPair(Smi, temporarySize, setTemporarySize, 3);
	AccessorPair(Smi, heapVarsSize, setHeapVarsSize, 4);
	AccessorPair(Smi, stackSize, setStackSize, 5);
	AccessorPair(StringOop, sourceText, setSourceText, 6);
	AccessorPair(SymbolOop, selector, setSelector, 7);
	AccessorPair(ClassOop, methodClass, setMethodClass, 8);
	AccessorPair(Smi, watch, setWatch, 9);

	static MethodOop new0(ObjectMemory &omem);

	void print(int in);
};

class BlockOopDesc : public OopOopDesc {
	static const int clsNstLength = 11;

    public:
	AccessorPair(ByteArrayOop, bytecode, setBytecode, 0);
	AccessorPair(ArrayOop, literals, setLiterals, 1);
	AccessorPair(Smi, argumentCount, setArgumentCount, 2);
	AccessorPair(Smi, temporarySize, setTemporarySize, 3);
	AccessorPair(Smi, heapVarsSize, setHeapVarsSize, 4);
	AccessorPair(Smi, stackSize, setStackSize, 5);
	AccessorPair(StringOop, sourceText, setSourceText, 6);
	AccessorPair(Oop, receiver, setReceiver, 7);
	AccessorPair(ArrayOop, parentHeapVars, setParentHeapVars, 8);
	AccessorPair(Smi, homeMethodContext, setHomeMethodContext, 9);
	/* This is either:
	 * A) an Association<Class, Block>;
	 * B) a HandlerCollection;
	 * C) an ensure: block handler; or
	 * D) an ifCurtailed: block handler.
	 * We differentiate between A/B v.s. C or D by the selector; we
	 * differentiate A and B by their isKindOf: response.
	 */
	AccessorPair(OopOop, handler, setHandler, 10);

	/**
	 * Allocates a new empty block.
	 */
	static BlockOop new0(ObjectMemory& omem);

	void print(int in);
};

class ContextOopDesc : public OopOopDesc {
	friend int execute(ObjectMemory &omem, ProcessOop proc,
	    volatile bool &interruptFlag) noexcept;

	static const int clsNstLength = 8;

    public:
	/**
	 * The previous BP, usually the one in which a message send invoked
	 * this context.
	 *
	 * n.b. in the case of a Block, as an optimisation, the (value[:])*
	 * method that used #blockInvoke is *not* the previous context; that
	 * context is effectively abolished, as it's of no real interest to
	 * anyone.
	 */
	Smi prevBP;
	/**
	 * The Method or Block this Context is running.
	 */
	OopOop methodOrBlock;
	/**
	 * If isBlockContext() true: BP of the context in which this block was
	 * instantiated; otherwise nil.
	 */
	Smi homeMethodBP;
	/**
	 * Convenience pointer to #methodOrBlock's bytecode array.
	 */
	ByteArrayOop bytecode;
	ArrayOop heapVars;
	ArrayOop parentHeapVars;
	Smi programCounter;
	Oop reg0;

	/**
	 * These allocate Contexts on the heap - not currently used.
	 */
	static ContextOop newWithBlock(ObjectMemory &omem, BlockOop aMethod);
	static ContextOop newWithMethod(ObjectMemory &omem, Oop receiver,
	    MethodOop aMethod);

	/* Initialise all fields that need to be (i.e. the SmiOops) */
	void init();
	void initWithBlock(ObjectMemory &omem, BlockOop aMethod);
	void initWithMethod(ObjectMemory &omem, Oop receiver,
	    MethodOop aMethod);

	inline BlockOop &block() { return methodOrBlock.as<BlockOop>(); }
	inline MethodOop &method() { return methodOrBlock.as<MethodOop>(); }
	Oop &regAt0(size_t num) { return m_oops[7 + num]; }

	bool isBlockContext();

	/** Full size of this context object in memory, in Oops. */
	size_t fullSize()
	{
		return m_size + (sizeof(MemOopDesc) / sizeof(Oop));
	}

	void print(int in);
};

#if 0
class LinkedListOopDesc : public OopOopDesc {
	public:
	static const int clsNstLength = 1;

	LinkOop firstLink;
	LinkOop lastLink;
};
#endif

class NativePointerOopDesc : public ByteArrayOopDesc {
public:
	static NativePointerOop new0(ObjectMemory &omem, void * pointer);

	void *&vns() { return (void*&)m_bytes; }
};

class ProcessOopDesc : public OopOopDesc {
	static const int clsNstLength = 8;

    public:
	enum State {
		kSuspended,
		kRunning,
		kWaiting,
		kDone,
	};

	ProcessOop link;
	Smi pid;
	StringOop name;
	ArrayOop stack;
	Smi bp; /* 1-based */
	Oop accumulator;
	Smi state;
	AssociationLinkOop events;

	static ProcessOop allocate(ObjectMemory &omem);

	ContextOop context() { return &stack->basicAt(bp.smi()); }
	ContextOop contextAt(size_t offset) { return &stack->basicAt(offset); }
};

class SchedulerOopDesc : public OopOopDesc {
	public:
	static const int clstNstLength = 3;

	ProcessOop runnable;
	ProcessOop waiting;
	ProcessOop curProc;

	/**
	 * Add a process to the runnable list. Process must NOT already be in
	 * the list.
	 */
	void addProcToRunnables(ProcessOop proc);
	/**
	 * Suspend the currently-running process. Places it into the #waiting
	 * list.
	 */
	void suspendCurrentProcess();
	/**
	 * Get the next runnable process, moving it to the last position in
	 * the runnable queue.
	 */
	ProcessOop getNextForRunning();
	/**
	 * Suspend a process.
	 *
	 * \pre proc is in the runnable list
	 */
	void suspendProcess(ProcessOop proc);
};

inline const char *
ClassOopDesc::nameCStr()
{
	return name->asCStr();
}

#endif /* OBJECTS_HH_ */
