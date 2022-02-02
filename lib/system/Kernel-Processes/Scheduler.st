Object subclass: Scheduler [
	"Schedules eco-friendly threads within a native thread."
| (Process)runnable "Head of list of runnable processes"
  (Process)waiting  "Head of list of processes not currently runnable"
  (Process)curProc  "Currently-running process." |

	allProcessesDo: aBlock [
		| temp |

		temp <- runnable.
		temp notNil whileTrue: [
			aBlock value: temp.
			temp <- temp nextLink
		].

		temp <- waiting.
		temp notNil whileTrue: [
			aBlock value: temp.
			temp <- temp nextLink
		].

		aBlock value: curProc.
	]

	disableInterrupts [
		<#disableInterrupts >
	]

	enableInterrupts [
		<#enableInterrupts >
	]

	(T) critical: ([^T]) aBlock <T> [
		| result |
		<#disableInterrupts >.
		result <- aBlock value.
		<#enableInterrupts >.
		^ result
	]

	(List<Semaphore>) waitForObjects: (List<Semaphore>)objects [
		"for each object: lock the object, check if it's ready
		  - if so then return it in the list (should we try to do an
		    'acquire' operation, i.e. `wait` on a semaphore here?)
		  - else add this process to that object's waiters queue"
		<#disableInterrupts >. "todo lock each object instead"
		objects do: [ :obj |
			obj acquire
		].
		<#enableInterrupts >.
	]

	yield [
		^ <#yield >.
	]

]