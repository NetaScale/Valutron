Object subclass: Context [
	| (Context | Integer)prevContext
	  (Method | Block)methodOrBlock
	  (Context) homeMethodContext
	  (ByteArray)bytecode
	  (Array)heapVars
	  (Array)parentHeapVars
	  (Integer)programCounter
	  (id)reg0 |

	shortName [
		| className |
		className <- reg0 class name asString.
		^ self isBlockContext ifTrue: [
			'[] in ', className, '>>', homeMethodContext methodOrBlock message
		] ifFalse: [
			className, '>>', methodOrBlock message
		]
	]

	homeMethodContext [
		^ homeMethodContext
	]

	receiver [
		^ reg0.
	]

	"old methods follow below"

	"class>>method: aMeth arguments: aVec temporaries: tVec [
		^ self new method: aMeth arguments: aVec temporaries: tVec
	]"

	"accessing"

	prevContext [
		^ prevContext
	]

	literals [
		"^ literals"
		^ 'fixme'
	]

	"arguments [
		 ^arguments
	]
	arguments: a [
		arguments <- a
	]

	temporaries [
		 ^ temporaries
	]"

	heapVars [
		 ^ heapVars
	]

	reg0 [
		 ^ reg0
	]

	bytecode [
		^ bytecode
	]

	methodOrBlock [
		^ methodOrBlock
	]

	isBlockContext [
		^ methodOrBlock class == Block
	]


	"at: key put: value [
		temporaries at: key put: value
	]"

	copy [
		^ super copy "temporaries: temporaries copy"
	]

	method: m [
		"method <- m"
		^ 'fixMe'
	]

	method: aMeth arguments: aVec temporaries: tVec [
		"method <- aMeth."
		^ 'fixMe'.
		"arguments <- aVec.
		temporaries <- tVec"
	]

	returnToBlock: bytePtr [
		" change the location we will return to, to execute a block"
		"<28 self bytePtr>"
		^ 0
	]

	"temporaries: t [
		temporaries <- t
	]"


]