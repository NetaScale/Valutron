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
		^ 55
	]
]

Object subclass: INITIAL [

	initial [ | fac |
		"fac <- [:n |
			n <= 1 
				ifTrue: [n]
				ifFalse: [n * (fac value: n - 1)]
			].
			^ fac value: 3 asLongInteger."
	^ 105 asLongInteger factorial printString
	"^ Tester blockRetWith: 1 and: 2 and: 3"
	"^ Tester blockRetWith: 1 and: 2 and: 3"
	"^ (20 fib)"
	"^ (10 asLongInteger + 2) printString"
	]
]

"9.90"
"9.07"
"10.06"
"9.65"
"9.18"