Object subclass: Semaphore [
	| (Integer)count (List)waiters |

	class>>new [
		^ self basicNew
			count: 0
			waiters: List new
	]

	class>>forMutualExclusion [
		^ self basicNew
			count: 1
			waiters: List new
	]

	count: cInt waiters: pList [
		count <- cInt.
		waiters <- pList
	]

	critical: aBlock [
		self wait.
		aBlock value.
		self signal
	]

	set: aNumber [
		count <- aNumber
	]

	signal [ | firstWaiter |
		scheduler critical: [
			count <- count + 1.
			(count <= 0) ifTrue: [
				firstWaiter <- waiters first value.
				waiters removeFirst.
				firstWaiter receiveEvent: self
			]
		]
	]

	"First checks if there are excess"
	wait [
		scheduler disableInterrupts.
		count <- count - 1.
		(count < 0) ifTrue: [
			waiters add: thisProcess.
			thisProcess state: 3. "waiting"
			scheduler enableInterrupts.
			scheduler yield
		] ifFalse: [ scheduler enableInterrupts ]
	]

	tryAcquire: anEventQueue [
		^ nil
	]
]