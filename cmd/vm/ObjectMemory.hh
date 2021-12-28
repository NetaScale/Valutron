#ifndef OBJECTMEMORY_HH_
#define OBJECTMEMORY_HH_

#include <stdexcept>
#include <err.h>

extern "C" {
#include "mps.h"
#include "mpstd.h"
}

#include "Oops.hh"

#define FATAL(...) errx(EXIT_FAILURE, __VA_ARGS__)
#define ALIGNMENT 8
#define ALIGN(size) (((size) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

class ObjectMemory {
	static uint32_t s_hashCounter;

	/** Shared global arena. */
	static mps_arena_t m_arena;
	/** Object format. */
	mps_fmt_t m_objFmt;
	/** AMC pool for MemOopDescs. */
	mps_pool_t m_amcPool;
	/** AMCZ pool for PrimOopDescs (leaf objects). */
	mps_pool_t m_amczPool;

	/** Allocation point for regular objects. */
	mps_ap_t m_objAP;
	/** Allocation point for leaf objects. */
	mps_ap_t m_leafAp;
	/** Root for this thread's stack */
	mps_root_t m_threadRoot;
	/** MPS thread representation. */
	mps_thr_t m_mpsThread;

	/** Generate a 24-bit number to be used as an object's hashcode. */
	static inline uint32_t getHashCode();

    public:
	static MemOop objNil;
	static MemOop objTrue;
	static MemOop objFalse;
	/* Indexed by string hash */
	static DictionaryOop objSymbolTable;
	/* Indexed by object hashcode */
	static DictionaryOop objGlobals;
	static MemOop objsmalltalk;
	static MemOop objUnused1;
	static MemOop objUnused2;
	static MemOop objUnused3;
	static MemOop objMinClass;
	static ClassOop clsObjectMeta;
	static ClassOop clsObject;
	static ClassOop clsSymbol;
	static ClassOop clsInteger;
	static ClassOop clsArray;
	static ClassOop clsByteArray;
	static ClassOop clsString;
	static ClassOop clsMethod;
	static ClassOop clsProcess;
	static ClassOop clsUndefinedObject;
	static ClassOop clsTrue;
	static ClassOop clsFalse;
	static ClassOop clsLink;
	static ClassOop clsDictionary;
	static ClassOop clsBlock;
	static ClassOop clsContext;
	static ClassOop clsSymbolTable;
	static ClassOop clsSystemDictionary;
	static ClassOop clsFloat;
	static ClassOop clsVM;
	static ClassOop clsChar;
	static ClassOop clsProcessor;
	static ClassOop clsNativeCode;
	static ClassOop clsNativePointer;

	ObjectMemory(void *stackMarker);

	/**
	 * Allocates an object composed of object pointers.
	 */
	template <class T> T newOopObj(size_t len);
	/**
	 * Allocates an object composed of bytes.
	 */
	template <class T> T newByteObj(size_t len);

	ClassOop findOrCreateClass(ClassOop superClass, std::string name);
	ClassOop lookupClass(std::string name);

	/**
	 * Performs a cold boot of the Object Manager. Essential objects are
	 * provisionally set up such that it is possible to compile code and
	 * define classes.
	 */
	void setupInitialObjects();
};

template <class T>
inline ClassOop &
OopRef<T>::isa()
{
	return isSmi() ? ObjectMemory::clsInteger : as<MemOop>()->isa();
}

template <class T>
inline ClassOop &
OopRef<T>::setIsa(ClassOop oop)
{
	return isSmi() ? throw std::runtime_error("Can't set SMI hashcode") :
	    as<MemOop>()->setIsa(oop);
}

static inline uint32_t
hash(uint32_t x)
{
	x = ((x >> 16) ^ x) * 0x119de1f3;
	x = ((x >> 16) ^ x) * 0x119de1f3;
	x = (x >> 16) ^ x;
	return x >> 8;
}

inline uint32_t
ObjectMemory::getHashCode()
{
	do {
		int x = hash(s_hashCounter++);
		if (x != 0)
			return x;
	} while (true);
}

template <class T>
T
ObjectMemory::newOopObj(size_t len)
{
	typename T::PtrType * obj;
	size_t size =  ALIGN(sizeof(MemOopDesc) + sizeof(Oop) * len);

	do {
		mps_res_t res = mps_reserve(((void **)&obj), m_objAP, size);
		if (res != MPS_RES_OK)
			FATAL("out of memory in newOopObj");
		obj->m_kind = MemOopDesc::kOops;
		obj->m_hash = getHashCode();
		obj->m_size = len;
	} while (!mps_commit(m_objAP, ((void *)obj), size));

	return obj;
}

template <class T>
T
ObjectMemory::newByteObj(size_t len)
{
	typename T::PtrType * obj;
	size_t size =  ALIGN(sizeof(MemOopDesc) + sizeof(uint8_t) * len);

	do {
		mps_res_t res = mps_reserve(((void **)&obj), m_objAP, size);
		if (res != MPS_RES_OK)
			FATAL("out of memory in newByteObj");
		obj->m_kind = MemOopDesc::kBytes;
		obj->m_hash = getHashCode();
		obj->m_size = len;
	} while (!mps_commit(m_objAP, ((void *)obj), size));

	return obj;
}

#endif /* OBJECTMEMORY_HH_ */
