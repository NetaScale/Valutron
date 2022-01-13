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

Object subclass: INITIAL [

	initial [ | fac |
		"fac <- [:n |
			n <= 1 
				ifTrue: [n]
				ifFalse: [n * (fac value: n - 1)]
			].
			^ fac value: 3 asLongInteger."
	^ (15 asLongInteger factorial printString) "(fac value: (20 asLongInteger)) printString"
		"^ 2 + 3"
		"^ false and: [ true ]"

		"fac <- 1.
		[ fac < 5 ] whileTrue: [ fac <- fac + 1. 42].
		^ fac"
		"^ 258 radix: 16"
	]
]
"real    0m8.311s"

"21: 362395969008173056"
"20: 127058998962946048 / REAL:  2.432902e+18 "
"19: 121645100408832000"
"18: 6402373705728000"