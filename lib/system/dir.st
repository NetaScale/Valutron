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
	^ 65 asLongInteger factorial printString
	"^ 5852689 radix: 16"
	"^ (30 fib)"
	]
]

"Completed in 8.22121"
"Completed in 8.34563"