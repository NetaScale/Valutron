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
			temp <- temp link
		].

		temp <- waiting.
		temp notNil whileTrue: [
			aBlock value: temp.
			temp <- temp link
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

	yield [
		^ <#yield >.
	]

]