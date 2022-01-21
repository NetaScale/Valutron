#ifndef OOPS_HH_
#define OOPS_HH_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

extern "C" {
#include "mps.h"
}

#define VT_tagBits 3
#define VT_tagMask 7
#define VT_tag(x) (((intptr_t)x) & VT_tagMask)
#define VT_isPtr(x) (!VT_tag (x))
#define VT_isSmi(x) (VT_tag (x) == 1)
#define VT_intValue(x) (((intptr_t)x) >> VT_tagBits)
#define VT_fromInt(iVal) ((void *) (((iVal) << VT_tagBits) | 1))

class ObjectMemory;

class OopDesc;
class ArrayOopDesc;
class ByteOopDesc;
class ByteArrayOopDesc;
class BlockOopDesc;
class CacheOopDesc;
class CharOopDesc;
class ClassOopDesc;
class ContextOopDesc;
class DictionaryOopDesc;
class FloatOopDesc;
class LinkOopDesc;
class NativeCodeOopDesc;
class SmiOopDesc;
class StringOopDesc;
class SymbolOopDesc;
class MemOopDesc;
class MethodOopDesc;
class OopOopDesc;
class ProcessOopDesc;
class SchedulerOopDesc;

template <class T> class OopRef;

/* clang-format off */
typedef OopRef <OopDesc> 		Oop;
typedef OopRef  <OopDesc>		SmiOop;
typedef OopRef  <FloatOopDesc> 		FloatOop;
typedef OopRef  <MemOopDesc> 		MemOop;
typedef OopRef   <OopOopDesc> 		OopOop;
typedef OopRef    <ArrayOopDesc>	ArrayOop;
typedef OopRef    <BlockOopDesc>	BlockOop;
typedef OopRef    <CacheOopDesc>	CacheOop;
typedef OopRef    <CharOopDesc>		CharOop;
typedef OopRef    <ClassOopDesc>	ClassOop;
typedef OopRef    <NativeCodeOopDesc>	NativeCodeOop;
typedef OopRef    <ContextOopDesc>	ContextOop;
typedef OopRef    <DictionaryOopDesc>	DictionaryOop;
typedef OopRef    <LinkOopDesc> 	LinkOop;
typedef OopRef    <MethodOopDesc>	MethodOop;
typedef OopRef    <ProcessOopDesc>	ProcessOop;
typedef OopRef    <SchedulerOopDesc>	SchedulerOop;
typedef OopRef    <StringOopDesc>	StringOop;
typedef OopRef    <SymbolOopDesc>	SymbolOop;
typedef OopRef   <ByteOopDesc>		ByteOop;
typedef OopRef    <ByteArrayOopDesc>	ByteArrayOop;
/* clang-format on */

template <class T> class OopRef {
    //protected:
    public:
	friend class OopRef<OopDesc>;

	enum Tag {
		kPtr = 0,
		kSmi = 1,
	};

	T *m_ptr;

    public:
	typedef T PtrType;

	inline OopRef()
	    : m_ptr(NULL) {};
	inline OopRef(int64_t smi)
	    : m_ptr((T *)VT_fromInt(smi)) {};
	inline OopRef(void *ptr)
	    : m_ptr((T *)ptr) {};

	static inline OopRef nil() { return OopRef(); }

	inline bool isPtr() const { return VT_isPtr(m_ptr); }
	inline bool isSmi() const { return VT_isSmi(m_ptr); }
	inline bool isNil() const { return m_ptr == 0; }
	inline int64_t smi() const { return VT_intValue(m_ptr); }
	template <typename OT> inline OT & as() { return reinterpret_cast<OT&>(*this); }

	inline ClassOop &isa();
	inline ClassOop &setIsa(ClassOop oop);

	void print(size_t in);

	inline uint32_t hashCode()
	{
		return isSmi() ? smi() : as<MemOop>()->hashCode();
	}

	template <typename OT> inline bool operator==(const OT &other)
	{
		return !operator!=(other);
	}
	template <typename OT> inline bool operator!=(const OT &other)
	{
		return other.m_ptr != m_ptr;
	}
	T *operator->() const { return m_ptr; }
	inline T &operator*() const { return *m_ptr; }
	inline operator Oop() const { return m_ptr; }
} __attribute__((packed));

class OopDesc {

};

class Klass {
public:
	static Klass klass;

	enum Format {
		kBasic,
		kClass,
	} format = kBasic;

	Klass(Format format = kBasic) : format(format) {};

	virtual std::ostream &print(std::ostream &os) const {
		return os << "nothing";
	}
};

extern "C" int execute(ObjectMemory &omem, ProcessOop proc,
    uintptr_t timeslice) noexcept;
static mps_res_t scanGlobals(mps_ss_t ss, void *p, size_t s);

class MemOopDesc : public OopDesc {
    protected:
	friend class ObjectMemory;
	friend class OopRef<OopDesc>;
	friend class ProcessOopDesc;

	friend int execute(ObjectMemory &omem, ProcessOop proc,
	    uintptr_t timeslice) noexcept;
	friend mps_res_t scanGlobals(mps_ss_t ss, void *p, size_t s);

	enum Kind {
		kPad,
		kFwd,
		kBytes,
		kOops,
		kStack,
		kStackAllocatedContext, /* musn't be scanned */
		kBlock, /* needs skipping of Context pointer */
	};

	union {
		ClassOop m_isa;
		MemOop m_fwd;
	};
	struct {
		int32_t m_size;	 /**< number of bytes or Oops */
		Kind m_kind : 8; /**< kind of object - byte or Oops bearer? */
		int32_t m_hash : 24; /**< hashcode (in place of address) */
	};
	union {
		Oop m_oops[0];
		uint8_t m_bytes[0];
	};

    public:
	/**
	 * Return the size of the object's von Neumann space in Oops or bytes.
	 */
	size_t size() { return m_size; }
	inline ClassOop &isa() { return m_isa; }
	inline ClassOop &setIsa(ClassOop oop) { return m_isa = oop; }

	int32_t hashCode() const { return m_hash; }
	inline int32_t setHashCode(int32_t code) { return m_hash = code; }

	static mps_res_t mpsScan(mps_ss_t ss, mps_addr_t base,
	    mps_addr_t limit);
	static mps_addr_t mpsSkip(mps_addr_t base);
	static void mpsFwd(mps_addr_t old, mps_addr_t newAddr);
	static mps_addr_t mpsIsFwd(mps_addr_t addr);
	static void mpsPad(mps_addr_t addr, size_t size);
};

class OopOopDesc : public MemOopDesc {
    public:
	/** Return a pointer to the object's von Neumann space. */
	Oop *vns() { return m_oops; }

	/**
	 * Return a reference to the Oop value at /a index.
	 * 0-based indexing!
	 */
	Oop &basicAt0(size_t idx) { return m_oops[idx]; };

	/**
	 * At index /a index, put Oop value /a value.
	 * 0-based indexing!
	 * @return Reference to the value placed at the index.
	 */
	Oop &basicAtPut0(size_t i, Oop val) { return m_oops[i] = val; };

	Oop &basicAt(size_t idx) { return m_oops[idx - 1]; }
	Oop &basicAtPut(size_t i, Oop val) { return m_oops[i - 1] = val; }
};

class ByteOopDesc : public MemOopDesc {
    public:
	/** Return a pointer to the object's von Neumann space. */
	uint8_t *vns() { return m_bytes; }

	/**
	 * Return a reference to the byte value at /a index.
	 * 0-based indexing!
	 */
	uint8_t &basicAt0(size_t i) { return m_bytes[i]; };

	/**
	 * At index /a index, put byte value /a value.
	 * 0-based indexing!
	 * @return Reference to value placed at the index.
	 */
	uint8_t &basicAtPut0(size_t i, uint8_t val) { return m_bytes[i] = val; };

	uint8_t &basicAt(size_t idx) { return m_bytes[idx - 1]; }
	uint8_t &basicAtPut(size_t i, uint8_t val) { return m_bytes[i-1] = val; }
};

class FloatOopDesc : public ByteOopDesc {
    public:
	/** Return a pointer to the object's von Neumann space. */
	double &floatValue() { return *(double *)m_bytes; }
};

/**
 * \defgroup Object descriptors
 * @{
 */

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
	SmiOop version;
	MethodOop method;
	CacheOop next;

	static CacheOop newWithSelector(ObjectMemory &omem, SymbolOop value);
};

class CharOopDesc : public OopOopDesc {
    public:
	AccessorPair(SmiOop, value, setValue, 0);

	static CharOop newWith(ObjectMemory &omem, intptr_t value);
};

class LinkOopDesc : public OopOopDesc {
    public:
	AccessorPair(Oop, one, setOne, 0);
	AccessorPair(Oop, two, setTwo, 1);
	AccessorPair(LinkOop, nextLink, setNextLink, 2);

	void print(int in);

	static LinkOop newWith(ObjectMemory &omem, Oop a, Oop b);
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

	SymbolOop name;
	ClassOop superClass;
	DictionaryOop methods;
	SmiOop nstSize;
	ArrayOop nstVars;
	DictionaryOop dictionary;
	Klass *klass;

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
	AccessorPair(SmiOop, argumentCount, setArgumentCount, 2);
	AccessorPair(SmiOop, temporarySize, setTemporarySize, 3);
	AccessorPair(SmiOop, heapVarsSize, setHeapVarsSize, 4);
	AccessorPair(SmiOop, stackSize, setStackSize, 5);
	AccessorPair(StringOop, sourceText, setSourceText, 6);
	AccessorPair(SymbolOop, selector, setSelector, 7);
	AccessorPair(ClassOop, methodClass, setMethodClass, 8);
	AccessorPair(SmiOop, watch, setWatch, 9);

	static MethodOop new0(ObjectMemory &omem);

	void print(int in);
};

class BlockOopDesc : public OopOopDesc {
	static const int clsNstLength = 11;

    public:
	AccessorPair(ByteArrayOop, bytecode, setBytecode, 0);
	AccessorPair(ArrayOop, literals, setLiterals, 1);
	AccessorPair(SmiOop, argumentCount, setArgumentCount, 2);
	AccessorPair(SmiOop, temporarySize, setTemporarySize, 3);
	AccessorPair(SmiOop, heapVarsSize, setHeapVarsSize, 4);
	AccessorPair(SmiOop, stackSize, setStackSize, 5);
	AccessorPair(StringOop, sourceText, setSourceText, 6);
	AccessorPair(Oop, receiver, setReceiver, 7);
	AccessorPair(ArrayOop, parentHeapVars, setParentHeapVars, 8);
	AccessorPair(ContextOop, homeMethodContext, setHomeMethodContext, 9);
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
	    uintptr_t timeslice) noexcept;

	static const int clsNstLength = 8;

    public:
	/**
	 * The previous context, usually the one in which a message send invoked
	 * this context.
	 *
	 * n.b. in the case of a Block, as an optimisation, the (value[:])*
	 * method that used #blockInvoke is *not* the previous context; that
	 * context is effectively abolished, as it's of no real interest to
	 * anyone.
	 */
	ContextOop previousContext;
	/**
	 * The Method or Block this Context is running.
	 */
	OopOop methodOrBlock;
	/**
	 * If isBlockContext() true: The Context in which this block was
	 * instantiated; otherwise nil.
	 */
	ContextOop homeMethodContext;
	/**
	 * Convenience pointer to #methodOrBlock's bytecode array.
	 */
	ByteArrayOop bytecode;
	ArrayOop heapVars;
	ArrayOop parentHeapVars;
	SmiOop programCounter;
	Oop reg0;

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

	static ContextOop newWithBlock(ObjectMemory &omem, BlockOop aMethod);
	static ContextOop newWithMethod(ObjectMemory &omem, Oop receiver,
	    MethodOop aMethod);
};

class ProcessOopDesc : public OopOopDesc {
	static const int clsNstLength = 5;

    public:
	ContextOop context;
	ArrayOop stack;
	SmiOop stackIndex; /* 1-based */
	Oop accumulator;
	ProcessOop link;

	static ProcessOop allocate(ObjectMemory &omem);
};

class SchedulerOopDesc : public OopOopDesc {
	public:
	static const int clstNstLength = 2;

	ProcessOop runnable;
	ProcessOop waiting;
	//ProcessOop current;

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
	 * Suspend a process. Must not be the current process.
	 */
	void suspendProcess(ProcessOop proc);
};

inline const char *
ClassOopDesc::nameCStr()
{
	return name->asCStr();
}

/**
 * @}
 */

#endif /* OOPS_HH_ */
