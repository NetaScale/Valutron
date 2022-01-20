#ifndef OBJECTMEMORY_HH_
#define OBJECTMEMORY_HH_

#include <stdexcept>

#include "Oops.hh"
#include "Config.hh"

#if VT_GC == VT_GC_BOEHM
# include "gc.h"
#define VT_GCNAME "Boehm LibGC"
# define calloc(x, y) GC_malloc(x * y)
#elif VT_GC == VT_GC_MPS
# define VT_GCNAME "Ravenbrook MPS"
extern "C" {
# include "mps.h"
# include "mpstd.h"
}
#else
# define VT_GCNAME "None (System Malloc)"
#endif

#define FATAL(...) { fprintf(stderr, __VA_ARGS__); abort(); }
#define ALIGNMENT 16
#define ALIGN(size) (((size) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

class ObjectMemory {
	static uint32_t s_hashCounter;

	/** Shared global arena. */
	static mps_arena_t m_arena;
	/** Chain */
	mps_chain_t m_chain;
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
	/** Root for globals */
	mps_root_t m_globalRoot;
	/** MPS thread representation. */
	mps_thr_t m_mpsThread;

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

	/** the metaclass hierarchy */
	static ClassOop clsObjectClass;
	static ClassOop clsObject;
#if 0
	static ClassOop clsBehavior;
	static ClassOop clsClassDescription;
	static ClassOop clsClass;
	static ClassOop clsMetaclass;
#endif

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
	static ClassOop clsCharacter;
	static ClassOop clsProcessor;
	static ClassOop clsNativeCode;
	static ClassOop clsNativePointer;

	static const char * binOpStr[13];
	static SymbolOop symBin[13];

	ObjectMemory(void *stackMarker);

	/**
	 * Allocates an object composed of object pointers.
	 */
	template <class T> T newOopObj(size_t len,
	    MemOopDesc::Kind = MemOopDesc::kOops);
	/**
	 * Allocates an object composed of bytes.
	 */
	template <class T> T newByteObj(size_t len);
	/**
	 * Copies any object.
	 */
	template <class T> T copyObj(MemOop obj);

	/** Generate a 24-bit number to be used as an object's hashcode. */
	static inline uint32_t getHashCode();

	ClassOop findOrCreateClass(ClassOop superClass, std::string name);
	ClassOop lookupClass(std::string name);

	/**
	 * Performs a cold boot of the Object Manager. Essential objects are
	 * provisionally set up such that it is possible to compile code and
	 * define classes.
	 */
	void setupInitialObjects();

	void poll();
};

static inline uint32_t
hash(uint32_t x)
{
	x = ((x >> 16) ^ x) * 0x119de1f3;
	x = ((x >> 16) ^ x) * 0x119de1f3;
	x = (x >> 16) ^ x;
	return x >> 8;
}

template <class T>
inline ClassOop &
OopRef<T>::isa()
{
	return isSmi() ? ObjectMemory::clsInteger :
	    isNil()    ? ObjectMemory::clsUndefinedObject :
			       as<MemOop>()->isa();
}

template <class T>
inline ClassOop &
OopRef<T>::setIsa(ClassOop oop)
{
	if (isSmi()) {
		printf("Can't set SMI hashcode\n");
		abort();
	} else
		return as<MemOop>()->setIsa(oop);
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
ObjectMemory::newOopObj(size_t len, MemOopDesc::Kind kind)
{
	typename T::PtrType * obj;
	size_t size =  ALIGN(sizeof(MemOopDesc) + sizeof(Oop) * len);

#if VT_GC == VT_GC_MPS
	do {
		mps_res_t res = mps_reserve(((void **)&obj), m_objAP, size);
		if (res != MPS_RES_OK)
			FATAL("out of memory in newOopObj");
		obj->m_isa = Oop::nil().as<ClassOop>();
		obj->m_kind = kind;
		obj->m_hash = getHashCode();
		obj->m_size = len;
		memset(obj->m_oops, 0, len * sizeof(Oop));
	} while (!mps_commit(m_objAP, ((void *)obj), size));
#else
	obj = (typename T::PtrType*)calloc(1, size);
		obj->m_kind = kind;
		obj->m_hash = getHashCode();
		obj->m_size = len;
#endif

	return obj;
}

template <class T>
T
ObjectMemory::newByteObj(size_t len)
{
	typename T::PtrType * obj;
	size_t size =  ALIGN(sizeof(MemOopDesc) + sizeof(uint8_t) * len);

#if VT_GC == VT_GC_MPS
	do {
		mps_res_t res = mps_reserve(((void **)&obj), m_objAP, size);
		if (res != MPS_RES_OK)
			FATAL("out of memory in newByteObj");
		obj->m_isa = Oop::nil().as<ClassOop>();
		obj->m_kind = MemOopDesc::kBytes;
		obj->m_hash = getHashCode();
		obj->m_size = len;
		memset(obj->m_bytes, 0, len);
	} while (!mps_commit(m_objAP, ((void *)obj), size));
#else
	obj = (typename T::PtrType*)calloc(1, size);
		obj->m_kind = MemOopDesc::kBytes;
		obj->m_hash = getHashCode();
		obj->m_size = len;
#endif


	return obj;
}

template <class T>
T
ObjectMemory::copyObj(MemOop oldObj)
{
	typename T::PtrType * obj;
	size_t size =  ALIGN(sizeof(MemOopDesc) +
	    (oldObj->m_kind == MemOopDesc::kBytes ?
	    sizeof(uint8_t) :
	    sizeof (Oop)) * oldObj->size());

#if VT_GC == VT_GC_MPS
	do {
		mps_res_t res = mps_reserve(((void **)&obj), m_objAP, size);
		if (res != MPS_RES_OK)
			FATAL("out of memory in copyObj");
		memcpy(obj, &*oldObj, size);
		obj->m_hash = getHashCode();
	} while (!mps_commit(m_objAP, ((void *)obj), size));
#else
	obj = (typename T::PtrType*)calloc(1, size);
	memcpy(obj, &*oldObj, size);
	obj->m_hash = getHashCode();
#endif

	return obj;
}

#endif /* OBJECTMEMORY_HH_ */
