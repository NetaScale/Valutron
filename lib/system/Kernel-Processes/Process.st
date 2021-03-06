Link subclass: Process [
|
 (Integer)pid		"Process ID"
 (String)name		"Process name"
 (Array<id>)stack	"Stack of inlined context objects."
 (Integer)stackIndex	"1-based index into :stack of topmost stack frame."
 (id)accumulator	"The accumulator register."
 (Integer)state		"Process state - 0 suspended, 1 active, 2 done, 3+ waiting"
|

	"creation"
	class>>newWithReceiver: anObject selector: aSymbol args: anArray  [
		^ <#newProcessMessage anObject aSymbol anArray>
	]

	class>>newWithBlock: aBlock [
		^ <#procNewFork aBlock>
	]

	"accessors and enqueries"

	pid [
		^ pid
	]

	name [
		^ name
	]

	name: aString [
		name <- aString
	]

	context [
		^ StackFrame new initWithProcess: self bp: stackIndex
	]

	stack [
		^ stack
	]

	state [
		^state
	]

	state: anInteger [
		state <- anInteger.
	]

	"management"

	receiveEvent: anEvent [
		state <- 1.
		self resume
	]

	"unsuspends a process. adds it to the scheduler runqueue of the current system thread"
	resume [
		^ <#procResume self>
	]

	"below is old code"

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
			stackIndex: 10
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
		"(overflowed isNil and: [(stack size > 8192)])
			ifTrue: [
				overflowed <- true.
				VM error: 'process stack overflowed']."
		"<19 self> ifTrue: [] ifFalse: [ self terminate ]."
		^ 42
	]

	method: x  [
		stack at: 5 put: x.
	]

	stack: sArray stackIndex: sInt linkPointer: lInt [
		stack <- sArray.
		stackIndex <- sInt
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
		"overflowed notNil ifTrue: [
			^ self ]."
		scheduler yield.
		more <- 8.
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