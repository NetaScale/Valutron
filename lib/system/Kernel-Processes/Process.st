Object subclass: Process [
	| stack stackTop linkPointer overflowed |

	class>>context: cObj startAt: sInt [
		^ self new context: cObj startAt: sInt
	]

	class>>new [
		| sArray |
		sArray <- Array new: 50.
		sArray at: 2 put: 0. "previous link"
		sArray at: 4 put: 1. "return point"
		sArray at: 6 put: 1. "bytecode counter"
		^ self basicNew
			stack: sArray
			stackTop: 10
			linkPointer: 2
	]


	context [
		^ stack at: 3
	]

	context: ctx [
		stack at: 3 put: ctx.
	]

	context: cObj startAt: sInt [
		stack at: 3 put: cObj.
		stack at: 6 put: sInt "starting bytecode value"
	]

	execute  [
		" execute for time slice, terminating if all over "
		(overflowed isNil and: [(stack size > 8192)])
			ifTrue: [
				overflowed <- true.
				VM error: 'process stack overflowed'].
		<19 self> ifTrue: [] ifFalse: [ self terminate ].
	]

	method: x  [
		stack at: 5 put: x.
	]

	resume [
		" resume current process "
		scheduler addProcess: self
	]

	stack: sArray stackTop: sInt linkPointer: lInt [
		stack <- sArray.
		stackTop <- sInt.
		linkPointer <- lInt
	]

	startAt: x [
		stack at: 6 put: x. "starting bytecode value"
	]

	terminate [
		" kill current process "
		scheduler removeProcess: self. scheduler yield.
	]

	trace [		| more link m r s |
		" first yield scheduler, forceing store of linkPointer"
		overflowed notNil ifTrue: [
			^ self ].
		scheduler yield.
		more <- 8.
		link <- linkPointer.
		link <- stack at: link+1.
		" then trace back chain "
		[ more > 0 and: [link ~= 0] ] whileTrue:
			[ m <- stack at: link+3.
			  m notNil
				ifTrue: [ s <- m signature, ' ('.
			  		  r <- stack at: link+2.
			  		  (r to: link - 1) do:
						[:x | s <- s, ' ',
							(stack at: x) class asString].
					  (s, ')') print ].
			  more <- more - 1.
			  link <- stack at: link ]
	]


]