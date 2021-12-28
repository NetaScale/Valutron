#ifndef OOPS_HH_
#define OOPS_HH_

#include <cstddef>
#include <cstdint>
#include <string>

extern "C" {
#include "mps.h"
}

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

template <class T> class OopRef {
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
	inline OopRef()
	    : m_ptr(NULL) {};
	inline OopRef(int64_t smi)
	    : m_smi(smi)
	    , m_tag(kSmi) {};
	inline OopRef(OopDesc *ptr)
	    : m_ptr((T *)ptr) {};

	inline bool isPtr() const { return m_tag == kPtr; }
	inline bool isSmi() const { return m_tag == kSmi; }
	inline bool isNil() const { return m_ptr == 0; }
	inline int64_t smi() const { return m_smi; }
	template <typename OT> inline OT as() { return OT(m_ptr); };

	template <typename OT> inline bool operator==(const OT &other)
	{
		return !operator==(other);
	}
	template <typename OT> inline bool operator!=(const OT &other)
	{
		return other->m_ptr == m_ptr;
	}
	inline T *operator->() const { return m_ptr; }
	inline T &operator*() const { return *m_ptr; }
};

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

class MemOopDesc {
	protected:
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
	uint8_t *vns();
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

class DictionaryOopDesc : public OopOopDesc {
    public:
	/**
	 * Inserts /a value under /a key under the hash /a hash.
	 */
	void insert(intptr_t hash, Oop key, Oop value);

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
	void symbolInsert(SymbolOop key, Oop value);

	Oop symbolLookup(SymbolOop aSymbol);

	/*
	 * Looks up the value associated with whichever symbol carries string
	 * value aString.
	 */
	Oop symbolLookup(std::string aString);

	/*
	 * Namespace functions.
	 */
	ClassOop findOrCreateClass(ClassOop superClass, std::string name);
	/* Create or find a subnamespace with the given name. Name may be
	 * colonised.
	 */
	DictionaryOop subNamespaceNamed(std::string name);
	/* Look up a symbol, searching superspaces if necessary. */
	Oop symbolLookupNamespaced(std::string aString);

#pragma mark creation

	static DictionaryOop newWithSize(size_t numBuckets);

#pragma mark misc
	void print(int in);
};

/**
 * @}
 */
#endif /* OOPS_HH_ */
