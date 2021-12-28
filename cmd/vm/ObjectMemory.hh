#ifndef OBJECTMEMORY_HH_
#define OBJECTMEMORY_HH_

extern "C" {
#include "mps.h"
#include "mpstd.h"
}

#include "Oops.hh"

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
	static Oop objNil;
	static Oop objTrue;
	static Oop objFalse;
	/* Indexed by string hash */
	static DictionaryOop objSymbolTable;
	/* Indexed by value */
	static DictionaryOop objGlobals;
	static Oop objsmalltalk;
	static Oop objUnused1;
	static Oop objUnused2;
	static Oop objUnused3;
	static Oop objMinClass;
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

#endif /* OBJECTMEMORY_HH_ */
