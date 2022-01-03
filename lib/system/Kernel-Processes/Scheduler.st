Object subclass: Scheduler [
	| notdone processList currentProcess |

	class>>new [
		^ self basicNew
			notdone: true
			processList: Set new
			currentProcess: nil
	]


	addProcess: aProcess [
		" add a process to the process list "
		processList add: aProcess
	]

	critical: aBlock [
		"set time slice counter high to insure bytecodes are
		executed before continuing "
		<53 10000>.
		aBlock value.
		"then yield processor "
		<53 0> "."
	]

	currentProcess [
		" return the currently executing process "
		^ currentProcess
	]

	initialize [	| string |
		<2>.
		string <- smalltalk getPrompt: '> '.
		string isNil ifTrue: [
			'EOF' logChunk. "FIXME: '''EOF'''"
			notdone <- false ]
		ifFalse: [
			(string size > 0) ifTrue: [
				string logChunk.
				echoInput ifTrue: [
					string print ].
				[ string value print ] fork ] ]
	]

	notdone: nBool processList: pSet currentProcess: cProc [
		notdone <- nBool.
		processList <- pSet.
		currentProcess <- cProc
	]

	removeProcess: aProcess [
		" remove a given process from the process list "
		processList remove: aProcess "FIXME: ."
	]

	run [
		" run as long as process list is non empty "
		[ notdone ] whileTrue:
			[ processList size = 0 ifTrue: 
				[ self initialize ].
			  processList do: 
				[ :x | currentProcess <- x.
					x execute  ] ]
	]

	yield [
		" set time slice counter to zero, thereby
		yielding to next process "
		<53 0>
	]


]