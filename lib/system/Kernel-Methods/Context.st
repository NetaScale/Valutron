Object subclass: Context [
	| prevContext programCounter stackPointer receiver arguments
	  temporaries heapVars parentHeapVars stack bytecode methodOrBlock
	  homeMethodContext |

	class>>method: aMeth arguments: aVec temporaries: tVec [
		^ self new method: aMeth arguments: aVec temporaries: tVec
	]

	"accessing"

	prevContext [
		^ prevContext
	]

	literals [
		^ literals
	]

	arguments [
		 ^arguments
	]
	arguments: a [
		arguments <- a
	]

	temporaries [
		 ^ temporaries
	]

	heapVars [
		 ^ heapVars
	]

	stack [
		 ^ stack
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


	at: key put: value [
		temporaries at: key put: value
	]

	blockReturn [
		<18 self>
			ifFalse: [ ^ VM error: 
				'incorrect context for block return']
	]

	copy [
		^ super copy temporaries: temporaries copy
	]

	method: m [
		method <- m
	]

	method: aMeth arguments: aVec temporaries: tVec [
		method <- aMeth.
		arguments <- aVec.
		temporaries <- tVec
	]

	returnToBlock: bytePtr [
		" change the location we will return to, to execute a block"
		<28 self bytePtr>
	]

	temporaries: t [
		temporaries <- t
	]


]