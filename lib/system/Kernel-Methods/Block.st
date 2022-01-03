Object subclass: Block [
	| bytecode literals argCount temporariesSize heapVarsSize stackSize
	  sourceText receiver parentHeapVars homeContext |

	" accessing"
	homeContext [
		^ homeContext
	]

	checkArgumentCount: count [
		^ (argCount = count)
			ifTrue: [ true ]
			ifFalse: [ VM error:
				'wrong number of arguments passed to block'.
				false ]
	]

	fork [
		self newProcess resume
	]

	forkWith: args [
		(self newProcessWith: args) resume
	]

	newProcess [
		" create a new process to execute block "
		^ Process context: context startAt: bytePointer
	]

	newProcessWith: args [
		(self checkArgumentCount: args size)
			ifTrue: [ (1 to: args size) do: [:i |
				   context at: (argLoc + i - 1) 
					put: (args at: i)]].
		^ self newProcess
	]

	value [
		^ <160 self>
	]

	value: x [
		^ (self checkArgumentCount:  1)
			ifTrue: [ context at: argLoc put: x.
				  context returnToBlock: bytePointer ]
	]

	value: x value: y [
		^ (self checkArgumentCount: 2)
			ifTrue: [ context at: argLoc put: x.
				  context at: argLoc + 1 put: y.
				  context returnToBlock: bytePointer ]
	]

	value: x value: y value: z [
		^ (self checkArgumentCount:  3)
			ifTrue: [ context at: argLoc put: x.
				  context at: argLoc + 1 put: y.
				  context at: argLoc + 2 put: z.
				  context returnToBlock: bytePointer ]
	]

	whileFalse [
		self whileFalse: []
	]


	whileFalse: aBlock [
		[ self value not ] whileTrue: aBlock
	]

	whileTrue [
		self whileTrue: []
	]

	whileTrue: aBlock [
		( self value ) ifTrue:
			[ aBlock value. 
				self whileTrue: aBlock ]
	]

	nonLocalReturn: returnValue [
		"Description:
			Makes a non-local return of @p value to homeContext. Executes ensure/ifCurtailed: blocks along the way."

		| i prevContext tempHomeContext |

		i <- 1.
		tempHomeContext <- homeContext.
		prevContext <- thisContext.

		[
		  prevContext <- prevContext prevContext.
		  i <- i + 1.
		  (prevContext == homeContext) or: [ prevContext isNil] 
		] whileTrue.

		[ prevContext isBlockContext ]
			whileTrue: [
				homeContext <- prevContext methodOrBlock homeContext.

				[
				  i <- i + 1.
				  (prevContext <- prevContext prevContext) == homeContext 
					or: [ homeContext isNil ]
				] whileFalse 
			].
		
		"Now escape the method context. "
		prevContext <- prevContext prevContext.
		prevContext isNil ifTrue: [ <163 'PrevContext is nil'> ].
		<162 'Stack unwind completed. Activating return continuation:'>.
		<4 prevContext returnValue>
	]

	testBlock [
		 [ thisContext methodOrBlock nonLocalReturn: 55 
						  ] ensure: [ <162 'Hello!'> ] 
	]

	"category: Kernel-Exceptions"

	ensure: terminationBlock [
		| handler returnValue |
		"Description: 
			Evaluate the receiver and return its result. Immediately after successful evaluation of the receiver
			but before returning its result, evaluate terminationBlock. If abnormal termination of the 
			receiver occurs, terminationBlock is evaluated. In either case, the value returned from the
			evaluation of terminationBlock is discarded.

			Activation of an exception handler from within the receiver is not in and of itself an abnormal
			termination. However, if the exception handler for an exception that is not resumable results in
			termination of the receiver or if its handler block contains a return statement that results in
			abnormal termination of the receiver, then terminationBlock will be evaluated after evaluation
			of the exception handler.

			If an abnormal termination results in the termination of multiple blocks which were evaluated using
			either #ensure: or #ifCurtailed: the respective terminationBlocks will be executed in
			the reverse of the order in which the corresponding receiver blocks were evaluated."
		"Implementation notes:
			There are two means by which our terminationBlock may be executed: either the block returns a value
			to us and we then execute the terminationBlock, or a non-local return is invoked. Non-local returns
			reverse iterate through the context chain, executing all handlers associated with a selector of #ensure:
			or #ifCurtailed: if the receiver is a block, until they find their target context."

		handler <- terminationBlock.
		returnValue <- self value.
		terminationBlock value.
		^ returnValue
	]

	ifCurtailed: terminationBlock [
		| handler returnValue |
		"Description:
			Evaluate the receiver and return its result. If abnormal termination of the receiver occurs,
			terminationBlock is evaluated. The value returned from the evaluation of
			terminationBlock is discarded.

			Activation of an exception handler from within the receiver is not in and of itself an abnormal
			termination. However, if the exception handler for an exception that is not resumable results in
			termination of the receiver or if its handler block contains a return statement that results in
			abnormal termination of the receiver, then terminationBlock will be evaluated after evaluation
			of the exception handler.

			If an abnormal termination result in the termination of multiple blocks which were evaluated using
			either #ensure: or #ifCurtailed: the respective terminationBlocks will be executed in
			the reverse of the order in which the corresponding receiver blocks were evaluated."
		"Implementation notes:
			Implemented as per #ensure, except we don't execute our handler at all - a non-local return, however,
			will execute it."

		handler <- terminationBlock.
		returnValue <- self value.
		^ returnValue
	]

	on: selector do: action [
		"The receiver is evaluated such that if during its evaluation an exception corresponding to
		selector is signaled then action will be evaluated. The result of evaluating the receiver is
		returned.

		Before evaluating the receiver the current state of the exception environment is captured as the
		handler environment. Then a new exception handler is created with selector as its exception
		selector and action as its handler block. The new handler is pushed onto the exception
		environment.

		If evaluation of the receiver terminates normally then the exception environment is reset to the
		handler environment before returning to the sender of the #on:do: message.
		If signaling of an exception results in evaluation of action the evaluation will occur in the context
		of the handler environment. The argument to the action will be an object that conforms to the
		protocol <signaledException>."

		^ self value
	]


]