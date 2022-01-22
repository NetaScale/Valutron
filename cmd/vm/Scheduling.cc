#include <cassert>
#include <ev++.h>
#include <pthread.h>
#include <unistd.h>

#include "CPUThread.hh"
#include "ObjectMemory.hh"
#include "Objects.hh"

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
	} else {
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

void
SchedulerOopDesc::suspendProcess(ProcessOop proc)
{
	removeFromList(runnable, proc);
}

ProcessOop
SchedulerOopDesc::getNextForRunning()
{
	ProcessOop toRun = runnable;
	if (!toRun.isNil())
		removeFromList(runnable, toRun);
	return toRun;
}

ProcessOop
makeProc(ObjectMemory &omem, std::string sel)
{
	ClassOop initial = omem.lookupClass("INITIAL");
	assert(!initial.isNil());

	MethodOop start = initial->methods->symbolLookup(
	    SymbolOopDesc::fromString(omem, sel)).as<MethodOop>();
	assert(!start.isNil());

	ProcessOop firstProcess = ProcessOopDesc::allocate(omem);
	ContextOop ctx = (void *)&firstProcess->stack->basicAt0(0);
	ctx->initWithMethod(omem, Oop(), start);
	firstProcess->context = ctx;

	return firstProcess;
}

void CPUThreadPair::loopAcquire(EV_P) noexcept
{
	CPUThreadPair * self = (CPUThreadPair*)ev_userdata(EV_A);
	pthread_mutex_lock(&self->m_evLock);
}

void CPUThreadPair::loopRelease(EV_P) noexcept
{
	CPUThreadPair * self = (CPUThreadPair*)ev_userdata(EV_A);
	pthread_mutex_unlock(&self->m_evLock);
}

void CPUThreadPair::loopAwakeCb(ev::async &w, int revents)
{
	if(m_wantExit)
		w.loop.break_loop();
}

void CPUThreadPair::timeSliceCb(ev::timer &w, int revents)
{
	m_interruptFlag = true;
}

void *
CPUThreadPair::startEvThread(void *arg)
{
	return ((CPUThreadPair *)arg)->evLoop();
}

void *
CPUThreadPair::evLoop()
{
	pthread_mutex_lock(&m_evLock);
	m_loop.run();
	pthread_mutex_unlock(&m_evLock);
	return NULL;
}

void
CPUThreadPair::scheduleLoop()
{
	ClassOop cls = m_omem.lookupClass("Scheduler");
	SchedulerOop sched;

	assert(!cls.isNil());
	sched = m_omem.newOopObj<SchedulerOop>(SchedulerOopDesc::clstNstLength);

	sched.setIsa(cls);

	ProcessOop proc1 = makeProc(m_omem, "doStuff1"),
		   proc2 = makeProc(m_omem, "doStuff2");
	sched->addProcToRunnables(proc1);
	sched->addProcToRunnables(proc2);

	pthread_mutex_lock(&m_evLock);
	m_timeSliceTimer.again();
	m_loopWake.send();
	std::cout << "Timeslicing starts.\n";
	pthread_mutex_unlock(&m_evLock);

loop:
	ProcessOop proc = sched->getNextForRunning();
	if (proc.isNil()) {
		std::cout << "All processes finished\n";
		pthread_mutex_lock(&m_evLock);
		m_timeSliceTimer.stop();
		m_loopWake.send();
		std::cout << "Timeslicing stops.\n";
		pthread_mutex_unlock(&m_evLock);
		return;
	}
	if (execute(m_omem, proc, m_interruptFlag) == 0) {
		std::cout << "Process " << proc.m_ptr << " finished\n";
	} else {
		std::cout << "Returning process " << proc.m_ptr
			  << " to run-queue\n";
		std::cout << proc.m_ptr << " stack size "
			  << proc->stackIndex.smi() << "\n";
		sched->addProcToRunnables(proc);
	}
	m_interruptFlag = false;
	goto loop;
}

CPUThreadPair::CPUThreadPair(ObjectMemory &omem)
    : m_omem(omem), m_loop(ev::default_loop()), m_loopWake(m_loop), m_timeSliceTimer(m_loop)
{
	int r;

	m_interpThread = pthread_self();
	pthread_mutex_init(&m_evLock, 0);

	ev_set_userdata(m_loop, this);
	ev_set_loop_release_cb(m_loop, loopRelease, loopAcquire);
	m_loopWake.set<CPUThreadPair, &CPUThreadPair::loopAwakeCb> (this);
	m_loopWake.start();
	m_timeSliceTimer.set(0., 0.1);
	m_timeSliceTimer.set<CPUThreadPair, &CPUThreadPair::timeSliceCb>(this);

	r = pthread_create(&m_evThread, NULL, startEvThread, this);

	scheduleLoop();

	pthread_mutex_lock(&m_evLock);
	m_wantExit = true;
	m_loopWake.send();
	pthread_mutex_unlock(&m_evLock);

	pthread_join(m_evThread, NULL);

	std::cout << "CPU thread exiting.\n";
}