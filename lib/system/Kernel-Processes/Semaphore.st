Object subclass: Semaphore [
	| (Integer)count (List)processList |

	class>>new [
		^ self basicNew
			count: 0
			processList: List new
	]

	class>>forMutualExclusion [
		^ self basicNew
			count: 1
			processList: List new
	]

	count: cInt processList: pList [
		count <- cInt.
		processList <- pList
	]

	critical: aBlock [
		self wait.
		aBlock value.
		self signal
	]

	set: aNumber [
		count <- aNumber
	]

	signal [ | firstProc |
		scheduler critical: [
			count <- count + 1.
			(count <= 0) ifTrue: [
				firstProc <- processList first value.
				firstProc state: 1.
				firstProc resume.
				processList removeFirst
			]
		]
	]

	wait [
		scheduler disableInterrupts.
		count <- count - 1.
		(count < 0) ifTrue: [
			processList add: thisProcess.
			thisProcess state: 3. "waiting"
			scheduler enableInterrupts.
			scheduler yield
		] ifFalse: [ scheduler enableInterrupts ]
	]
]