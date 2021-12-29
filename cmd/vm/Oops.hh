#ifndef OOPS_HH_
#define OOPS_HH_

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

extern "C" {
#include "mps.h"
}

class ObjectMemory;

class OopDesc;
class ArrayOopDesc;
class ByteOopDesc;
class ByteArrayOopDesc;
class BlockOopDesc;
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
class ProcessorOopDesc;

template <class T> class OopRef;

/* clang-format off */
typedef OopRef <OopDesc> 		Oop;
typedef OopRef  <OopDesc>		SmiOop;
typedef OopRef  <FloatOopDesc> 		FloatOop;
typedef OopRef  <MemOopDesc> 		MemOop;
typedef OopRef   <OopOopDesc> 		OopOop;
typedef OopRef    <ArrayOopDesc>	ArrayOop;
typedef OopRef    <BlockOopDesc>	BlockOop;
typedef OopRef    <CharOopDesc>		CharOop;
typedef OopRef    <ClassOopDesc>	ClassOop;
typedef OopRef    <NativeCodeOopDesc>	NativeCodeOop;
typedef OopRef    <ContextOopDesc>	ContextOop;
typedef OopRef    <DictionaryOopDesc>	DictionaryOop;
typedef OopRef    <LinkOopDesc> 	LinkOop;
typedef OopRef    <MethodOopDesc>	MethodOop;
typedef OopRef    <ProcessOopDesc>	ProcessOop;
typedef OopRef    <ProcessorOopDesc>	ProcessorOop;
typedef OopRef    <StringOopDesc>	StringOop;
typedef OopRef    <SymbolOopDesc>	SymbolOop;
typedef OopRef   <ByteOopDesc>		ByteOop;
typedef OopRef    <ByteArrayOopDesc>	ByteArrayOop;
/* clang-format on */

template <class T> class OopRef {
    protected:
	friend class OopRef<OopDesc>;

	enum Tag {
		kPtr = 0,
		kSmi = 1,
	};

	union {
		struct {
			int64_t m_smi : 61;
			Tag m_tag : 3;
		};
		T *m_ptr;
	};

    public:
	typedef T PtrType;

	inline OopRef()
	    : m_ptr(NULL) {};
	inline OopRef(int64_t smi)
	    : m_smi(smi)
	    , m_tag(kSmi) {};
	inline OopRef(void *ptr)
	    : m_ptr((T *)ptr) {};

	inline bool isPtr() const { return m_tag == kPtr; }
	inline bool isSmi() const { return m_tag == kSmi; }
	inline bool isNil() const { return m_ptr == 0; }
	inline int64_t smi() const { return m_smi; }
	template <typename OT> inline OT as() { return OT(m_ptr); }

	inline ClassOop &isa(); //{ return isSmi() ? 0 : as<MemOop>()->isa(); }
	inline ClassOop &setIsa(ClassOop oop);

	void print(size_t in);

	uint32_t hashCode()
	{
		return isSmi() ? m_smi : as<MemOop>()->hashCode();
	}

	template <typename OT> inline bool operator==(const OT &other)
	{
		return !operator!=(other);
	}
	template <typename OT> inline bool operator!=(const OT &other)
	{
		return other.m_ptr != m_ptr;
	}
	inline T *operator->() const { return m_ptr; }
	inline T &operator*() const { return *m_ptr; }
	inline operator Oop() const { return m_ptr; }
};

class OopDesc {

};

class MemOopDesc : public OopDesc {
    protected:
	friend class ObjectMemory;
	friend class OopRef<OopDesc>;

	enum Kind {
		kOops,
		kBytes,
		kPad,
		kFwd,
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
	/** Return the size of the object's von Neumann space. */
	size_t size() { return m_size; }

	/**
	 * Return a reference to the Oop value at /a index.
	 * 0-based indexing!
	 */
	Oop &basicAt(size_t idx) { return m_oops[idx]; };

	/**
	 * At index /a index, put Oop value /a value.
	 * 0-based indexing!
	 * @return Reference to the value placed at the index.
	 */
	Oop &basicAtPut(size_t i, Oop val) { return m_oops[i] = val; };
};

class ByteOopDesc : public MemOopDesc {
    public:
	/** Return a pointer to the object's von Neumann space. */
	uint8_t *vns() { return m_bytes; }
	/** Return the size of the object's von Neumann space. */
	size_t size();

	/**
	 * Return a reference to the byte value at /a index.
	 * 0-based indexing!
	 */
	uint8_t &basicAt(size_t i) { return m_bytes[i]; };

	/**
	 * At index /a index, put byte value /a value.
	 * 0-based indexing!
	 * @return Reference to value placed at the index.
	 */
	uint8_t &basicAtPut(size_t i, uint8_t val) { return m_bytes[i] = val; };
};

/**
 * \defgroup Object descriptors
 * @{
 */

#define AccessorPair(Type, GetName, SetName, Index)                    \
	inline Type GetName() { return m_oops[Index].as<Type>(); }     \
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
	std::pair<Oop, Oop> findPairByFun(intptr_t hash, ExtraType extraVal,
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
	static const int clsInstLength = 6;

	AccessorPair(SymbolOop, name, setName, 0);
	AccessorPair(ClassOop, superClass, setSuperClass, 1);
	AccessorPair(DictionaryOop, methods, setMethods, 2);
	AccessorPair(SmiOop, nstSize, setNstSize, 3);
	AccessorPair(ArrayOop, nstVars, setNstVars, 4);
	AccessorPair(DictionaryOop, dictionary, setDictionary, 5);

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
	static const int clsNstLength = 12;

    public:
	AccessorPair(ContextOop, previousContext, setPreviousContext, 0);
	AccessorPair(SmiOop, programCounter, setProgramCounter, 1);
	AccessorPair(SmiOop, stackPointer, setStackPointer, 2);
	AccessorPair(Oop, receiver, setReceiver, 3);
	AccessorPair(ArrayOop, arguments, setArguments, 4);
	AccessorPair(ArrayOop, temporaries, setTemporaries, 5);
	AccessorPair(ArrayOop, heapVars, setHeapVars, 6);
	AccessorPair(ArrayOop, parentHeapVars, setParentHeapVars, 7);
	AccessorPair(ArrayOop, stack, setStack, 8);
	AccessorPair(ByteArrayOop, bytecode, setBytecode, 9);
	AccessorPair(OopOop, methodOrBlock, setMethodOrBlock, 10);
	AccessorPair(ContextOop, homeMethodContext, setHomeMethodContext, 11);

	/* Initialise all fields that need to be (i.e. the SmiOops) */
	void init();
	/* Fetch the next byte of bytecode, incrementing the program counter. */
	uint8_t fetchByte();

	/**
	 * Duplicate the top of the stack
	 */
	void dup();
	/**
	 * Push a value to the stack
	 */
	void push(Oop obj);
	/**
	 * Pop a value from the stack
	 */
	Oop pop();
	/**
	 * Fetch the value at the top of the stack
	 */
	Oop top();

	bool isBlockContext();

	void print(int in);

	static ContextOop newWithBlock(ObjectMemory &omem, BlockOop aMethod);
	static ContextOop newWithMethod(ObjectMemory &omem, Oop receiver,
	    MethodOop aMethod);
};

class ProcessOopDesc : public OopOopDesc {
	static const int clsNstLength = 4;

    public:
	AccessorPair(ContextOop, context, setContext, 0);

	static ProcessOop allocate();
};

/**
 * @}
 */

#endif /* OOPS_HH_ */