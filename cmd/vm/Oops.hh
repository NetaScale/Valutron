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
class NativePointerOopDesc;
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
typedef OopRef  <OopDesc>		Smi;
typedef OopRef  <FloatOopDesc> 		FloatOop;
typedef OopRef  <MemOopDesc> 		MemOop;
typedef OopRef   <OopOopDesc> 		OopOop;
typedef OopRef    <ArrayOopDesc>	ArrayOop;
typedef OopRef    <BlockOopDesc>	BlockOop;
typedef OopRef    <CacheOopDesc>	CacheOop;
typedef OopRef    <ClassOopDesc>	ClassOop;
typedef OopRef    <CharOopDesc>		CharOop;
typedef OopRef    <ContextOopDesc>	ContextOop;
typedef OopRef    <DictionaryOopDesc>	DictionaryOop;
typedef OopRef    <LinkOopDesc> 	LinkOop;
typedef OopRef    <MethodOopDesc>	MethodOop;
typedef OopRef    <ProcessOopDesc>	ProcessOop;
typedef OopRef    <SchedulerOopDesc>	SchedulerOop;
typedef OopRef   <ByteOopDesc>		ByteOop;
typedef OopRef    <ByteArrayOopDesc>	ByteArrayOop;
typedef OopRef     <StringOopDesc>	StringOop;
typedef OopRef     <SymbolOopDesc>	SymbolOop;
typedef OopRef     <NativePointerOopDesc> NativePointerOop;

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
    volatile bool &interruptFlag) noexcept;
static mps_res_t scanGlobals(mps_ss_t ss, void *p, size_t s);

class MemOopDesc : public OopDesc {
    protected:
	template <class T> friend class ObjectAllocator;
	friend class ObjectMemory;
	friend class OopRef<OopDesc>;
	friend class ProcessOopDesc;

	friend int execute(ObjectMemory &omem, ProcessOop proc,
	    volatile bool &interruptFlag) noexcept;
	friend mps_res_t scanGlobals(mps_ss_t ss, void *p, size_t s);

	enum Kind {
		kPad,
		kFwd,
		kBytes,
		kOops,
		kStack,
		kStackAllocatedContext, /* musn't be scanned */
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

#endif /* OOPS_HH_ */
