Collection subclass: IndexedCollection [

	addAll: aCollection [
		aCollection binaryDo: [:i :x | self at: i put: x ]
	]

	asArray [
		^ (Array new: self size) yourself; addAll: self; yourself
	]

	asDictionary [
		^ Dictionary new yourself; addAll: self; yourself
	]

	at: aKey [
		^ self at: aKey 
			ifAbsent: [ <2000>. VM error: 'index to at: illegal' ]
	]

	at: index ifAbsent: exceptionBlock [
		 ^ (self includesKey: index)
			ifTrue: [ self basicAt: index ]
			ifFalse: exceptionBlock
	]

	binaryInject: thisValue into: aBlock [     | last |
		last <- thisValue.
		self binaryDo: [:i :x | last <- aBlock value: last 
						value: i value: x].
		^ last
	]

	collect: aBlock [
		^ self binaryInject: Dictionary new
			into: [:s :i :x | s at: i put: (aBlock value: x).  s]
	]

	do: aBlock [
		self binaryDo: [:i :x | aBlock value: x ]
	]

	indexOf: aBlock [
		^ self indexOf: aBlock
			ifAbsent: [ VM error: 'index not found']
	]

	indexOf: aBlock ifAbsent: exceptionBlock [
		self binaryDo: [:i :x | (aBlock value: x)
				ifTrue: [ ^ i ] ].
		^ exceptionBlock value
	]

	keys [
		^ self binaryInject: Set new 
			into: [:s :i :x | s add: i ]
	]

	select: aBlock [
		^ self binaryInject: Dictionary new
			into: [:s :i :x | (aBlock value: x)
					ifTrue: [ s at: i put: x ]. s ]
	]

	values [
		^ self binaryInject: List new
			into: [:s :i :x | s add: x ]
	]


]