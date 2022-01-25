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
		| testa  testb file ctx method |
		testa <- 25.
		"26 fib."
		file <- File new initWithDescriptor: 1 mode: #w.
		file << 'PID    NAME                    METHOD'; cr.
		[ 24 fib. ^ 55 ] fork resume.
		scheduler critical: [
			scheduler allProcessesDo: [ :proc |
				ctx <- proc = thisProcess ifTrue: [
					thisContext
				] ifFalse: [
					proc context
				].
				ctx isNil ifFalse: [
					file << (proc pid printString copyTruncateOrPadTo: 6 with: $ ); << ' '.
					file << '<unnamed>               '.
					file << (ctx shortName copyTruncateOrPadTo: 40 with: $.); cr
				] ifTrue: [
					file << 'dead process' ; cr
				]
			]
		].
		"file << 34 fib printString ; cr.
		file << 60 asLongInteger factorial printString; cr."
	]

	doStuff2 [
		|newproc|
		newproc <- Process newWithReceiver: INITIAL new selector: #initial args: nil.
		newproc resume.
		(29 fib).
		(1 to: 2) do: [ :i |
		<#debugMsg ('dostuff2 done' printString)> ].
		^42
	]

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
	^ 5 asLongInteger factorial printString
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