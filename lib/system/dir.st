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
		(1 to: 2) do: [ :i |
		
		<#debugMsg ((34 fib) printString)> ]
	]


	doStuff2 [
		(1 to: 2) do: [ :i |
		<#debugMsg (70 asLongInteger factorial printString )> ]
	]

	initial [ | fac |
		"fac <- [:n |
			n <= 1
				ifTrue: [n]
				ifFalse: [n * (fac value: n - 1)]
			].
			^ fac value: 3 asLongInteger."
	<#debugMsg (9164713 radix:16)>.
	<#debugMsg (20 fib printString)>.
	<#debugMsg ((10 asLongInteger + 2) printString)>.
	<#debugMsg ((Tester blockRetWith: 1 and: 2 and: 3) printString)>.
	^ 10 asLongInteger factorial printString
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