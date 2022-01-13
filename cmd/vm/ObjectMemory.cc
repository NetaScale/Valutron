#include <cassert>

extern "C" {
#include "mps.h"
#include "mpsavm.h"
#include "mpscamc.h"
}

#include "ObjectMemory.hh"

mps_arena_t ObjectMemory::m_arena = NULL;
uint32_t ObjectMemory::s_hashCounter = 1;

MemOop ObjectMemory::objNil;
MemOop ObjectMemory::objTrue;
MemOop ObjectMemory::objFalse;
/* Indexed by string hash */
DictionaryOop ObjectMemory::objSymbolTable;
/* Indexed by value */
DictionaryOop ObjectMemory::objGlobals;
MemOop ObjectMemory::objsmalltalk;
MemOop ObjectMemory::objUnused1;
MemOop ObjectMemory::objUnused2;
MemOop ObjectMemory::objUnused3;
MemOop ObjectMemory::objMinClass;
ClassOop ObjectMemory::clsObjectClass;
ClassOop ObjectMemory::clsObject;
ClassOop ObjectMemory::clsSymbol;
ClassOop ObjectMemory::clsInteger;
ClassOop ObjectMemory::clsArray;
ClassOop ObjectMemory::clsByteArray;
ClassOop ObjectMemory::clsString;
ClassOop ObjectMemory::clsMethod;
ClassOop ObjectMemory::clsProcess;
ClassOop ObjectMemory::clsUndefinedObject;
ClassOop ObjectMemory::clsTrue;
ClassOop ObjectMemory::clsFalse;
ClassOop ObjectMemory::clsLink;
ClassOop ObjectMemory::clsDictionary;
ClassOop ObjectMemory::clsBlock;
ClassOop ObjectMemory::clsContext;
ClassOop ObjectMemory::clsSymbolTable;
ClassOop ObjectMemory::clsSystemDictionary;
ClassOop ObjectMemory::clsFloat;
ClassOop ObjectMemory::clsVM;
ClassOop ObjectMemory::clsCharacter;
ClassOop ObjectMemory::clsProcessor;
ClassOop ObjectMemory::clsNativeCode;
ClassOop ObjectMemory::clsNativePointer;

const char * ObjectMemory::binOpStr[13] = {
			 "+",
                         "-",
                         "<",
                         ">",
                         "<=",
                         ">=",
                         "=",
                         "~=",
                         "*",
                         "quo:",
                         "rem:",
                         "bitAnd:",
                         "bitXor:",
};
SymbolOop ObjectMemory::symBin[13];

#define FIXOOP(oop)                                         \
	if (MPS_FIX1(ss, &*oop)) {                          \
		if (oop.isPtr()) {                          \
			/* Untag */                         \
			mps_addr_t ref = &*oop;             \
			mps_res_t res = MPS_FIX2(ss, &ref); \
                                                            \
			if (res != MPS_RES_OK)              \
				return res;                 \
                                                            \
			oop = (OopDesc *)ref;               \
		}                                           \
	}

mps_res_t
MemOopDesc::mpsScan(mps_ss_t ss, mps_addr_t base, mps_addr_t limit)
{
	MPS_SCAN_BEGIN (ss) {
		while (base < limit) {
			MemOopDesc *obj = (MemOopDesc *)base;
			char *addr = (char *)base;

			switch (obj->m_kind)
			case kFwd: {
				base = addr + obj->m_size;
				break;

			case kPad:
				base = addr + obj->m_size;
				break;

			case kBytes:
				base = addr + ALIGN(sizeof(MemOopDesc) +
				    obj->m_size);
				break;

			case kOops: {
				for (int i = 0; i < obj->m_size; i++)
					FIXOOP(obj->m_oops[i]);
				base = addr + ALIGN(sizeof(MemOopDesc) +
				    sizeof(Oop) * obj->m_size);

				break;
			}

			default: {
				FATAL("Bad object");
			}
			}
		}
	}
	MPS_SCAN_END(ss);
	return MPS_RES_OK;
}

mps_addr_t
MemOopDesc::mpsSkip(mps_addr_t base)
{
	MemOopDesc *obj = (MemOopDesc *)base;
	char *addr = (char *)base;

	switch (obj->m_kind)
	case kFwd: {
		return addr + obj->m_size;
		break;

	case kPad:
		return addr + obj->m_size;
		break;

	case kBytes:
		return addr + ALIGN(sizeof(MemOopDesc) + obj->m_size);
		break;

	case kOops:
		return addr + ALIGN(sizeof(MemOopDesc) + sizeof(Oop) *
		    obj->m_size);

	default:
		FATAL("Bad object");
	}
}

void
MemOopDesc::mpsFwd(mps_addr_t old, mps_addr_t newAddr)
{
	MemOopDesc *obj = (MemOopDesc *)old;
	obj->m_fwd = (OopDesc *)newAddr;
	obj->m_size = (char *)mpsSkip(old) - (char *)old;
	obj->m_kind = kFwd;
}

mps_addr_t
MemOopDesc::mpsIsFwd(mps_addr_t addr)
{
	MemOopDesc *obj = (MemOopDesc *)addr;
	if (obj->m_kind == kFwd)
		return &*obj->m_fwd;
	else
		return NULL;
}

void
MemOopDesc::mpsPad(mps_addr_t addr, size_t size)
{
	MemOopDesc *obj = (MemOopDesc *)addr;
	obj->m_size = size;
	obj->m_kind = kPad;
}



ObjectMemory::ObjectMemory(void *stackMarker)
{
	mps_res_t res;

	if (m_arena == NULL) {
		MPS_ARGS_BEGIN (args) {
			MPS_ARGS_ADD(args, MPS_KEY_ARENA_SIZE,
			    32 * 1024 * 1024);
			MPS_ARGS_DONE(args);
			res = mps_arena_create_k(&m_arena, mps_arena_class_vm(),
			    args);
		}
		MPS_ARGS_END(args);
		if (res != MPS_RES_OK)
			FATAL("Couldn't create arena");
	}

	mps_message_type_enable(m_arena, mps_message_type_gc());
	mps_message_type_enable(m_arena, mps_message_type_gc_start());

	MPS_ARGS_BEGIN (args) {
		MPS_ARGS_ADD(args, MPS_KEY_FMT_ALIGN, ALIGNMENT);
		MPS_ARGS_ADD(args, MPS_KEY_FMT_SCAN, MemOopDesc::mpsScan);
		MPS_ARGS_ADD(args, MPS_KEY_FMT_SKIP, MemOopDesc::mpsSkip);
		MPS_ARGS_ADD(args, MPS_KEY_FMT_FWD, MemOopDesc::mpsFwd);
		MPS_ARGS_ADD(args, MPS_KEY_FMT_ISFWD, MemOopDesc::mpsIsFwd);
		MPS_ARGS_ADD(args, MPS_KEY_FMT_PAD, MemOopDesc::mpsPad);
		MPS_ARGS_DONE(args);
		res = mps_fmt_create_k(&m_objFmt, m_arena, args);
	}
	MPS_ARGS_END(args);
	if (res != MPS_RES_OK)
		FATAL("Couldn't create primitive format");

	/** Create AMCZ pool for primitives. */
	MPS_ARGS_BEGIN (args) {
#if 0
		MPS_ARGS_ADD(args, MPS_KEY_CHAIN, m_mpsChain);
#endif
		MPS_ARGS_ADD(args, MPS_KEY_FORMAT, m_objFmt);
		MPS_ARGS_ADD(args, MPS_KEY_ALIGN, ALIGNMENT);
		MPS_ARGS_DONE(args);
		res = mps_pool_create_k(&m_amczPool, m_arena, mps_class_amcz(),
		    args);
	}
	MPS_ARGS_END(args);
	if (res != MPS_RES_OK)
		FATAL("Couldn't create AMCZ pool");

	/** Create AMC pool for objects. */
	MPS_ARGS_BEGIN (args) {
#if 0
		MPS_ARGS_ADD(args, MPS_KEY_CHAIN, m_mpsChain);
#endif
		MPS_ARGS_ADD(args, MPS_KEY_FORMAT, m_objFmt);
		MPS_ARGS_ADD(args, MPS_KEY_ALIGN, ALIGNMENT);
		MPS_ARGS_DONE(args);
		res = mps_pool_create_k(&m_amcPool, m_arena, mps_class_amc(),
		    args);
	}
	MPS_ARGS_END(args);
	if (res != MPS_RES_OK)
		FATAL("Couldn't create AMC pool");

	res = mps_ap_create_k(&m_objAP, m_amcPool, mps_args_none);
	if (res != MPS_RES_OK)
		FATAL("Couldn't create allocation point!");

	res = mps_ap_create_k(&m_leafAp, m_amczPool, mps_args_none);
	if (res != MPS_RES_OK)
		FATAL("Couldn't create allocation point!");

	res = mps_thread_reg(&m_mpsThread, m_arena);
	if (res != MPS_RES_OK)
		FATAL("Couldn't register thread");

	res = mps_root_create_thread(&m_threadRoot, m_arena, m_mpsThread,
	    stackMarker);
	if (res != MPS_RES_OK)
		FATAL("Couldn't create root");
}

void
ObjectMemory::setupInitialObjects()
{
	static_assert(sizeof(void*) == 8);
	static_assert(sizeof(Oop) == sizeof(void*));

#define CreateObj(Name, Size) obj##Name = newOopObj<MemOop>(Size)
	CreateObj(Nil, 0);
	CreateObj(True, 0);
	CreateObj(False, 0);
	objSymbolTable = newOopObj<DictionaryOop>(1);
	objGlobals = newOopObj<DictionaryOop>(1);
	CreateObj(smalltalk, 0);

	clsSymbol = ClassOopDesc::allocateRawClass(*this);
	clsArray = ClassOopDesc::allocateRawClass(*this);

	objSymbolTable->basicAtPut0(0, ArrayOopDesc::newWithSize(*this, 3 * 53));
	objGlobals->basicAtPut0(0, ArrayOopDesc::newWithSize(*this, 3 * 53));

	objGlobals->symbolInsert(*this,
	    SymbolOopDesc::fromString(*this, "Symbol"), clsSymbol);
	objGlobals->symbolInsert(*this,
	    SymbolOopDesc::fromString(*this, "Array"), clsArray);

#define CreateClass(Name)                                        \
	cls##Name = ClassOopDesc::allocateRawClass(*this);           \
	cls##Name->setName(SymbolOopDesc::fromString(*this, #Name)); \
	objGlobals->symbolInsert(*this,                              \
	    SymbolOopDesc::fromString(*this, #Name), cls##Name)

	CreateClass(ObjectClass);
	CreateClass(Object);
	clsObject.setIsa(clsObjectClass);
	CreateClass(Integer);
	CreateClass(ByteArray);
	CreateClass(String);
	CreateClass(Method);
	CreateClass(Process);
	CreateClass(UndefinedObject);
	CreateClass(True);
	CreateClass(False);
	CreateClass(Link);
	CreateClass(Dictionary);
	CreateClass(Block);
	CreateClass(Context);
	CreateClass(SymbolTable);
	CreateClass(SystemDictionary);
	CreateClass(Float);
	CreateClass(VM);
	CreateClass(Character);
	CreateClass(NativeCode);
	CreateClass(NativePointer);

#define AddGlobal(name, obj)            				       \
	objGlobals->symbolInsert(*this, SymbolOopDesc::fromString(*this,       \
	    name), obj)

	AddGlobal("nil", objNil);
	AddGlobal("true", objTrue);
	AddGlobal("false", objFalse);

	objNil.setIsa(clsUndefinedObject);
	objTrue.setIsa(clsTrue);
	objFalse.setIsa(clsFalse);
	objSymbolTable.setIsa(clsDictionary);
	objGlobals.setIsa(clsSystemDictionary);

	for (int i = 0; i < sizeof(binOpStr) / sizeof(*binOpStr); i++)
		symBin[i] = SymbolOopDesc::fromString(*this, binOpStr[i]);
}