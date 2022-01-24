#ifndef OSTHREAD_HH_
#define OSTHREAD_HH_

#include <pthread.h>
#include <ev++.h>

#include "Oops.hh"

/**
 * Defines the pair of threads associated with each CPU thread of the
 * interpreter. One thread runs the scheduling loop; the other runs a libev
 * event loop, and communicates with the interpreter thread by setting the
 * interrupt flag.
 */
class CPUThreadPair {
	volatile bool m_interruptFlag = false;	/** pending VM interrupt? */
	bool m_otherInterruptFlag = false; /**< pending int if intr disabled */
	bool m_interruptsDisabled = false; /**< are interrupts disabled? */
	bool m_wantExit = false;	/**< causes loop to exit on next wake */

	ObjectMemory	&m_omem;	/**< Thread's object memory. */
	SchedulerOop	m_scheduler;	/**< Thread's Smalltalk Scheduler. */

	pthread_t	m_interpThread;	/**< Interpreter thread. */
	pthread_t	m_evThread;	/**< Event loop thread. */
	pthread_mutex_t m_evLock;	/**< Event loop lock. */

	ev::loop_ref	m_loop;		/**< The event loop. */
	ev::async	m_loopWake;	/**< Event loop awakener. */
	ev::timer m_timeSliceTimer;	/**< Timeslicer timer. */

	/** Release the loop lock. Called by libev before waiting. */
	static void loopRelease(EV_P) noexcept;
	/** Acquire the loop lock. Called by libev after waiting. */
	static void loopAcquire(EV_P) noexcept;
	/** Entry point of the event loop thread. */
	static void *startEvThread(void *arg);

	/**
	 * Callback for when #m_loopWake is invoked.
	 * If #m_wantExit is true, exits.
	 */
	void loopAwakeCb(ev::async &w, int revents);
	/**
	 * Timeslicer callback - sets #m_interruptFlag to let the interpreter
	 * know.
	 */
	void timeSliceCb(ev::timer &w, int revents);

	/** Lock the event loop. */
	void lockLoop();
	/** Unlock the event loop and notify it of changes. */
	void unlockLoop();

	/** Event loop. */
	void *evLoop();
	/** Interpreter loop - the scheduler. */
	void scheduleLoop();

	/**
	 * Raise an interrupt with the interpreter thread (unless interrupts are
	 * disabled; if so, the interrupt is stored for delivery after enabling
	 * interrupts again.
	 */
	void interrupt();

    public:
	//static thread_local CPUThreadPair *curpair;

	CPUThreadPair(ObjectMemory &omem);

	static CPUThreadPair *curpair();

	void disableInterrupts();
	void enableInterrupts();
};

#endif /* OSTHREAD_HH_ */
