#include <cassert>

#include "Oops.hh"
#include "ObjectMemory.hh"

template <typename T>
void
addToListLast(T &list, T obj)
{
	if (list.isNil())
		list = obj;
	else {
		T last = list;
		while (!last->link.isNil())
			last = last->link;
		last->link = obj;
	}
}

template <typename T>
void
removeFromList(T &list, T obj)
{
	if (list == obj) {
		list = list->link;
	}
	else {
		T node = list;
		while (node->link != obj) {
			assert(!node->link.isNil());
			node = node->link;
		}

		node->link = node->link->link;
	}

	obj->link = T::nil();
}

void
SchedulerOopDesc::addProcToRunnables(ProcessOop proc)
{
	assert(proc->link.isNil());
	addToListLast(runnable, proc);
}

void
SchedulerOopDesc::suspendCurrentProcess()
{
}

void SchedulerOopDesc::suspendProcess(ProcessOop proc)
{
	removeFromList(runnable, proc);
}


ProcessOop SchedulerOopDesc::getNextForRunning()
{
	ProcessOop toRun = runnable;
	if (!toRun.isNil())
		removeFromList(runnable, toRun);
	return toRun;
}

ProcessOop makeProc(ObjectMemory &omem, std::string sel)
{
	ClassOop initial = ObjectMemory::objGlobals->symbolLookup(
	    SymbolOopDesc::fromString(omem, "INITIAL")).as<ClassOop>();
	assert(!initial.isNil());

	MethodOop start = initial->methods->symbolLookup(SymbolOopDesc::fromString(omem,
	    sel)).as<MethodOop>();
	assert(!start.isNil());

	ProcessOop firstProcess = ProcessOopDesc::allocate(omem);
	ContextOop ctx = (void*)&firstProcess->stack->basicAt0(0);
	ctx->initWithMethod(omem, Oop(),start);
	firstProcess->context = ctx;

	return firstProcess;
}

void run(ObjectMemory & omem)
{
	ClassOop cls = ObjectMemory::objGlobals->symbolLookup(
	    SymbolOopDesc::fromString(omem, "Scheduler")).as<ClassOop>();
	SchedulerOop sched;

	assert(!cls.isNil());
	sched = omem.newOopObj<SchedulerOop>(SchedulerOopDesc::clstNstLength);

	sched.setIsa(cls);

	ProcessOop proc1 = makeProc(omem, "doStuff1"), proc2 = makeProc(omem, "doStuff2");
	sched->addProcToRunnables(proc1);
	sched->addProcToRunnables(proc2);

	loop:
	ProcessOop proc = sched->getNextForRunning();
	if (proc.isNil()) {
			std::cout <<"All processes finished\n";
			return;
		}
	if (execute(omem, proc, 2000000) == 0) {
		std::cout << "Process "<< proc.m_ptr << " finished\n";
	} else {
		std::cout <<"Returning process " << proc.m_ptr << " to run-queue\n";
		std::cout << proc.m_ptr << " stack size " << proc->stackIndex.smi() << "\n";
		sched->addProcToRunnables(proc);
	}
	goto loop;
}