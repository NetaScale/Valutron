Object subclass: EventQueue [
	| (LinkedList)events (Process)waiter (LinkedList)eventSources |

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
			waiter state: 1.
			waiter resume
		]
	]

	getEvent [
		^ events removeFirst value
	]

	wait [	| acquired |
		events size == 0 ifFalse: [ ^ self ].
		eventSources do: [ :source |
			((source value tryAcquire: self) isNil) ifTrue: [
				nil alreadyGot
			]
		].
		events size == 0 ifTrue: [
			"do the waiting"
			scheduler critical: [ thisProcess state: 3. waiter <- thisProcess ].
			scheduler yield
		]
	]
]