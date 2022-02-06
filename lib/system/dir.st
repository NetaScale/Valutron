@include 'Kernel-Objects/dir.st'
@include 'Numeric-Magnitudes/dir.st'
@include 'Collection/dir.st'
@include 'Collections-Support/dir.st'
@include 'Collections-Text/dir.st'
@include 'Kernel-Exceptions/dir.st'
@include 'Kernel-Methods/dir.st'
@include 'Kernel-Native/dir.st'
@include 'Kernel-Processes/dir.st'
@include 'Numeric-Numbers/dir.st'
@include 'Arch-Unix/dir.st'

Object subclass: Tester [
	class>>doNothing [
		| athing |
		^ 40
	]
	class>>blockRetWith: a and: b and: c [
		| no |
		no <- self doNothing.
		"<#dumpVariable a>.
		<#dumpVariable b>.
		<#dumpVariable c>."
		^ a + c - b
	]
]

Object subclass: INITIAL [
	doStuff1 [
		"(1 to: 1) do: [ :i |
		<#debugMsg ((34 fib) printString)> ]"
		| file ctx newblk sem evq |
		"26 fib."
		evq <- EventQueue new.
		sem <- Semaphore forMutualExclusion.
		file <- File new initWithDescriptor: 1 mode: #w.
		"newblk <- [ file << (32 fib printString); cr. ^ 55 ] fork.
		newblk name: 'fib block'.
		newblk resume.
		newblk <- Process newWithReceiver: 95 asLongInteger selector: #factorial args: nil.
		newblk name: 'factorial'.
		newblk resume.
		scheduler critical: [
			file << 'PID    NAME                    METHOD'; cr.
			scheduler allProcessesDo: [ :proc |
				ctx <- proc = thisProcess ifTrue: [
					thisContext
				] ifFalse: [
					proc context
				].
				ctx isNil ifFalse: [
					file << (proc pid printString copyTruncateOrPadTo: 6 with: $ ); << ' '.
					(proc name isNil) ifTrue: [
						file << '<unnamed>               '
					] ifFalse: [
						file << (proc name copyTruncateOrPadTo: 24 with: $ )
					].
					file << (ctx shortName copyTruncateOrPadTo: 40 with: $.); cr
				] ifTrue: [
					file << 'dead process' ; cr
				]
			]
		]."

		newblk <- [ true whileTrue: [
			sem critical: [
				24 fib.
				file << 'a'.
				file << 46 asLongInteger factorial printString.
				file << 'b'.
				file cr
				]
			]
		] fork.
		newblk name: 'fib_1'.
		newblk resume.

		"newblk <- [ true whileTrue: [
			sem critical: [
			24 fib.
			file << 'A'.
			file << 46 asLongInteger factorial printString.
			file << 'B'.
			file cr
			]
			]
		] fork.
		newblk name: 'a'.
		newblk resume."

		evq addSource: sem.

		newblk <- [ true whileTrue: [ | evs |
				evs <- evq wait.
				24 fib.
				file << 'Bravo'.
				file << 46 asLongInteger factorial printString.
				file << 'Charlie'.
				file cr.
				sem signal
			]
		] fork.
		newblk name: 'fib_2'.
		newblk resume.

		24 fib.

		ctx <- LinkedList new.
		ctx add: (ValueLink value: 15).
		file << ctx first value printString; cr.

		scheduler critical: [
			file << 'PID    NAME                    METHOD'; cr.
			scheduler allProcessesDo: [ :proc |
				ctx <- proc = thisProcess ifTrue: [
					thisContext
				] ifFalse: [
					proc context
				].
				ctx isNil ifFalse: [
					file << (proc pid printString copyTruncateOrPadTo: 6 with: $ ); << ' '.
					(proc name isNil) ifTrue: [
						file << '<unnamed>               '
					] ifFalse: [
						file << (proc name copyTruncateOrPadTo: 24 with: $ )
					].
					file << (ctx shortName copyTruncateOrPadTo: 40 with: $.); cr
				] ifTrue: [
					file << 'dead process' ; cr
				]
			]
		].

		"file << 34 fib printString ; cr.
		file << 60 asLongInteger factorial printString; cr."
	]

	"doStuff2 [
		|newproc|
		newproc <- Process newWithReceiver: INITIAL new selector: #initial args: nil.
		newproc name: 'integral'.
		newproc resume.
		(30 fib).
		(1 to: 2) do: [ :i |
		<#debugMsg ('dostuff2 done' printString)> ].
		^42
	]"

	initial [ | fac |
		"fac <- [:n |
			n <= 1
				ifTrue: [n]
				ifFalse: [n * (fac value: n - 1)]
			].
			^ fac value: 3 asLongInteger."
	"<#debugMsg (9164713 radix:16)>.
	<#debugMsg (20 fib printString)>.
	<#debugMsg ((10 asLongInteger + 2) printString)>.
	<#debugMsg ((Tester blockRetWith: 1 and: 2 and: 3) printString)>."
	^ 55 asLongInteger factorial printString
	"^ Tester blockRetWith: 1 and: 2 and: 3"
	"^ Tester blockRetWith: 1 and: 2 and: 3"
	"^ (20 fib)"
	]
]

"9.90"
"9.07"
"10.06"
"9.65"
"9.18"