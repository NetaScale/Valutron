
Collection<T> subclass: LinkedList<"(Linkable)" T> [
	| (T)firstLink (T)lastLink |
	"An intrusive singly-linked list - stores a chain of elements, with each
	 element holding a pointer to the next element. Elements must implement
	 protocol Linkable."
	"Origin: Smalltalk-80 v2; parts from PDST."


	"methodsFor: 'accessing'"
	(T)first [
		"Answer the first link;  create an error if the receiver is empty."

		self emptyCheck.
		^firstLink
	]

	(T)last [
		"Answer the last link;  create an error if the receiver is empty."

		self emptyCheck.
		^lastLink
	]

	(Integer)size [
		"Answer how many elements the receiver contains."
		| tally |
		tally <- 0.
		self do: [:each | tally <- tally + 1].
		^tally
	]

	"methodsFor: 'testing'"
	(Boolean)isEmpty [
		^firstLink == nil
	]

	"methodsFor: 'adding'"
	add: (T)aLink [
		"Add aLink to the end of the receiver's list."

		^self addLast: aLink
	]

	addAll: aValue [
		"Add all values in iterable aValue to the end of this list."
		aValue do: [:x | self add: x ]
	]

	addFirst: (T)aLink [
		"Add aLink to the beginning of the receiver's list."

		self isEmpty ifTrue: [lastLink <- aLink].
		aLink nextLink: firstLink.
		firstLink <- aLink.
		^aLink
	]

	addLast: (T)aLink [
		"Add aLink to the end of the receiver's list."

		self isEmpty
			ifTrue: [firstLink <- aLink]
			ifFalse: [lastLink nextLink: aLink].
		lastLink <- aLink.
		^aLink
	]

	"methodsFor: 'removing'"
	(TRet)remove: (T)aLink ifAbsent: ([^TRet]) aBlock <TRet> [
		"Remove aLink from the receiver.  If it is not there, answer the result of
		evaluating aBlock."

		| tempLink |
		aLink == firstLink
			ifTrue: [firstLink <- aLink nextLink.
					aLink == lastLink
						ifTrue: [lastLink <- nil]]
			ifFalse: [tempLink <- firstLink.
					[tempLink == nil ifTrue: [^aBlock value].
					tempLink nextLink == aLink]
						whileFalse: [tempLink <- tempLink nextLink].
					tempLink nextLink: aLink nextLink.
					aLink == lastLink
						ifTrue: [lastLink <- tempLink]].
		aLink nextLink: nil.
		^aLink
	]

	(T)removeFirst [
		"Remove the first element.  If the receiver is empty, cause an error;
		otherwise answer the removed element.  Using the sequence addFirst:/removeFirst
		causes the receiver to behave as a stack; using addLast:/removeFirst causes the
		receiver to behave as a queue."

		| oldLink |
		self emptyCheck.
		oldLink <- firstLink.
		firstLink == lastLink
			ifTrue: [firstLink <- nil. lastLink <- nil]
			ifFalse: [firstLink <- oldLink nextLink].
		oldLink nextLink: nil.
		^oldLink
	]

	(T)removeLast [
		"Remove the receiver's last element.  If the receiver is empty, cause an error;
		otherwise answer the removed element.  Using addLast:/removeLast causes the
		receiver to behave as a stack; using addFirst:/removeLast causes the receiver to
		behave as a queue."

		| oldLink aLink |
		self emptyCheck.
		oldLink <- lastLink.
		firstLink == lastLink
			ifTrue: [firstLink <- nil. lastLink <- nil]
			ifFalse: [aLink <- firstLink.
					[aLink nextLink == oldLink] whileFalse:
						[aLink <- aLink nextLink].
					aLink nextLink: nil.
					lastLink <- aLink].
		oldLink nextLink: nil.
		^oldLink
	]

	"methodsFor: 'enumerating'"
	do: ([^id, T])aBlock [
		| aLink |
		aLink <- firstLink.
		[aLink == nil] whileFalse:
			[aBlock value: aLink.
			aLink <- aLink nextLink]
	]


	"LST"

	"add: aValue ordered: aBlock [
		(links isNil)
			ifTrue: [ self addFirst: aValue]
			ifFalse: [ links <- links add: aValue
					whenFalse: aBlock ]
	]

	collect: aBlock [
		^ self inject: self class new
		       into: [:x :y | x add: (aBlock value: y). x ]
	]

	reject: aBlock [
		^ self select: [:x | (aBlock value: x) not ]
	]

	reverseDo: aBlock [
		(links notNil)
			ifTrue: [ links reverseDo: aBlock ]
	]

	select: aBlock [
		^ self inject: self class new
		       into: [:x :y | (aBlock value: y)
					ifTrue: [x add: y]. x]
	]"
]