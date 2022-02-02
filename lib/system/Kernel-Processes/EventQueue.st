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
		waiter state == 3 ifTrue: [
			waiter state: 1.
			waiter resume
		] ifFalse: [
			VM error: 'Unexpected waiter state ', state asString
		]
	]

	wait [	| acquired |
		eventSources do: [ :source |
			((source value tryAcquire: self) isNil) ifFalse: [
				VM error: 'Event source is already acquirable '.
				nil failNow
			] ifTrue: [
				<#debugMsg 'Acquiring event'>.
				nil failPlease
			]
		]
	]
]