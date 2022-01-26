#include <cassert>
#include <unistd.h>

extern "C" {
#include "mps.h"
#include "mpsavm.h"
#include "mpscamc.h"
#include "mpscams.h"
}

#include "ObjectMemory.inl.hh"
#include "Objects.hh"

mps_arena_t ObjectMemory::m_arena = NULL;
uint32_t ObjectMemory::s_hashCounter = 1;

#define X(TYPE, NAME) TYPE ObjectMemory::NAME;
OMEM_STATICS
#undef X

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

#define FIXOOP(oop)                                                           \
	if (oop.isPtr() && MPS_FIX1(ss, &*oop)) { \
		/* Untag */                                                   \
		mps_addr_t ref = &*oop;                                       \
		mps_res_t res = MPS_FIX2(ss, &ref);                           \
                                                                              \
		if (res != MPS_RES_OK)                                        \
			return res;                                           \
                                                                              \
		oop = (OopDesc *)ref;                                         \
	}

#define ELEMENTSOF(X) (sizeof X / sizeof *X)

static mps_res_t
scanGlobals(mps_ss_t ss, void *p, size_t s)
{
	MPS_SCAN_BEGIN (ss) {
#define X(TYPE, NAME) FIXOOP(ObjectMemory::NAME)
		OMEM_STATICS
#undef X
		for (int i = 0; i < ELEMENTSOF(ObjectMemory::symBin); i++)
			FIXOOP(ObjectMemory::symBin[i]);

	}
	MPS_SCAN_END(ss);
	return MPS_RES_OK;
}

mps_res_t
MemOopDesc::mpsScan(mps_ss_t ss, mps_addr_t base, mps_addr_t limit)
{
	char *addr = (char *)base;
	MemOopDesc * prev;

	MPS_SCAN_BEGIN (ss) {
	loop:
		MemOopDesc *obj = (MemOopDesc *)addr;

		switch (obj->m_kind)
		case kFwd: {
			addr = addr + obj->m_size;
			break;

		case kPad:
			addr = addr + obj->m_size;
			break;

		case kBytes:
			FIXOOP(obj->isa());
			addr = addr + ALIGN(sizeof(MemOopDesc) + obj->m_size);
			break;

		case kOops: {
			FIXOOP(obj->isa());
			for (int i = 0; i < obj->m_size; i++)
				FIXOOP(obj->m_oops[i]);
			addr = addr + ALIGN(sizeof(MemOopDesc) + sizeof(Oop) *
			    obj->m_size);
			break;
		}

		case kStack: {
			ContextOopDesc *ctx = (ContextOopDesc *)&obj->m_oops[0];
			char *end = ((char *)obj + obj->m_size * sizeof(Oop));

			FIXOOP(obj->isa())

			while (!ctx->m_isa.isNil()) {
				FIXOOP(ctx->isa());

				for (int i = 0; i < ctx->m_size ; i++) {
					FIXOOP(ctx->basicAt0(i));
				}

				ctx = (ContextOopDesc *) ((char *)ctx +
				    sizeof(MemOopDesc) + ctx->m_size *
				    sizeof(Oop));
				if ((char *)ctx > end)
					break;
			}

			addr = addr + ALIGN(sizeof(MemOopDesc) + obj->m_size *
			    sizeof(Oop));
			break;
		}

		default: {
			FATAL("mpsScan: Bad object!!\n");
		}
		}

		if ((char*)obj == addr)
			sleep(1);

		if (addr < limit)
			goto loop;
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
	case kStack:
		return addr + ALIGN(sizeof(MemOopDesc) + sizeof(Oop) *
		    obj->m_size);

	case kStackAllocatedContext:
	default:
		FATAL("mpsSkip: Bad object!!");
	}
}

void
MemOopDesc::mpsFwd(mps_addr_t old, mps_addr_t newAddr)
{
	MemOopDesc *obj = (MemOopDesc *)old;
	obj->m_size = (char *)mpsSkip(old) - (char *)old;
	obj->m_fwd = (OopDesc *)newAddr;
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

#if 1
	mps_gen_param_s gen_params[] = {
	{ 16838, 0.9 },
	{ 16838, 0.4 },
	};

	res = mps_chain_create(&m_chain, m_arena,
                       sizeof(gen_params) / sizeof(gen_params[0]),
                       gen_params);
	if (res != MPS_RES_OK) FATAL("Couldn't create chain");
#endif

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
#if 1
		MPS_ARGS_ADD(args, MPS_KEY_CHAIN, m_chain);
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
#if 1
		MPS_ARGS_ADD(args, MPS_KEY_CHAIN, m_chain);
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

	ObjectAllocator::init(m_amcPool, m_amczPool);

	res = mps_root_create(&m_globalRoot, m_arena, mps_rank_exact(), 0,
	    scanGlobals, NULL, 0);
	if (res != MPS_RES_OK)
		FATAL("Couldn't create globals root");
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
	cls##Name->name = SymbolOopDesc::fromString(*this, #Name); \
	objGlobals->symbolInsert(*this,                              \
	    SymbolOopDesc::fromString(*this, #Name), cls##Name)

	CreateClass(ObjectClass);
	CreateClass(Object);
	clsObject.setIsa(clsObjectClass);
	clsObjectClass.setIsa(clsObjectClass);
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
	CreateClass(StackFrame);
	CreateClass(SymbolTable);
	CreateClass(SystemDictionary);
	CreateClass(Float);
	CreateClass(VM);
	CreateClass(Character);
	CreateClass(NativePointer);
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

ClassOop ObjectMemory::lookupClass(std::string name)
{
	return objGlobals->symbolLookup(SymbolOopDesc::fromString(*this, name)).
		as<ClassOop>();
}


void
ObjectMemory::poll()
{
	mps_message_type_t type;

	while (mps_message_queue_type(&type, m_arena)) {
		mps_message_t message;
		mps_bool_t b;
		b = mps_message_get(&message, m_arena, type);
		assert(b); /* we just checked there was one */

		if (type == mps_message_type_gc_start()) {
			printf("Collection started.\n");
			printf("  Why: %s\n",
			    mps_message_gc_start_why(m_arena, message));
			printf("  Clock: %lu\n",
			    (unsigned long)mps_message_clock(m_arena, message));

		} else if (type == mps_message_type_gc()) {
			size_t live = mps_message_gc_live_size(m_arena, message);
			size_t condemned = mps_message_gc_condemned_size(m_arena,
			    message);
			size_t not_condemned =
			    mps_message_gc_not_condemned_size(m_arena, message);
			printf("Collection finished.\n");
			printf("    live %lu\n", (unsigned long)live);
			printf("    condemned %lu\n", (unsigned long)condemned);
			printf("    not_condemned %lu\n",
			    (unsigned long)not_condemned);
			printf("    clock: %lu\n",
			    (unsigned long)mps_message_clock(m_arena, message));

		} else if (type == mps_message_type_finalization()) {
#if 0
			mps_addr_t port_ref;
			obj_t port;
			mps_message_finalization_ref(&port_ref, arena, message);
			port = port_ref;
#endif

		} else {
			printf("Unknown message from MPS!\n");
			abort();
		}

		mps_message_discard(m_arena, message);
	}
}