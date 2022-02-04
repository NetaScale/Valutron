Object subclass: EventQueue [
	| (LinkedList)events (Process)waiter (LinkedList)eventSources |
	"An EventQueue (or EvQueue or EvQ for short) allows multiple event
	 sources to be multiplexed, so that a process can wait till one is
	 ready"

	class>>new [
		^ super new init
	]

	init [
		eventSources <- LinkedList new.
		events <- LinkedList new.
	]

	addSource: aSource [
		eventSources add: (ValueLink value: aSource)
	]

	"Receive an event. Expects to be called with VM interrupts disabled."
	receiveEvent: anEvent [
		events addLast: (ValueLink value: anEvent).
		(waiter notNil and: [ waiter state == 3 ]) ifTrue: [
			scheduler critical: [
				waiter state: 1.
				waiter resume.
				waiter <- nil
			]
		]
	]

	getEvent [
		^ events removeFirst value
	]

	wait [	| acquired oldEvents |
		events size == 0 ifFalse: [ ^ self ].
		eventSources do: [ :source |
			((source value tryAcquire: self) isNil) ifTrue: [
				nil alreadyGot
			]
		].
		events size == 0 ifTrue: [
			scheduler critical: [ thisProcess state: 3. waiter <- thisProcess ].
			scheduler yield.
			scheduler critical: [ oldEvents <- events. events <- LinkedList new ].
			^ oldEvents
		]
	]
]