Magnitude subclass: Collection [

	< coll [
		(coll respondsTo: #includes:)
			ifFalse: [ ^ VM error:
				  'collection compared to non collection'].
		self do: [:x | ((self occurrencesOf: x) < 
			(coll occurrencesOf: x))ifFalse: [ ^ false ]].
		coll do: [:x | (self includes: x) ifFalse: [ ^ true ]].
		^ false
	]

	= coll [
		self do: [:x | (self occurrencesOf: x) = 
				(coll occurrencesOf: x) ifFalse: [ ^ false ] ].
		^ true
	]

	asArray [		| newArray i |
		newArray <- Array new: self size.
		i <- 0.
		self do: [:x | i <- i + 1. newArray at: i put: x].
		^ newArray
	]

	asByteArray [	| newArray i |
		newArray <- ByteArray new size: self size.
		i <- 0.
		self do: [:x | i <- i + 1. newArray at: i put: x].
		^ newArray
	]

	asSet [
		^ Set new addAll: self
	]

	asString [
		^ self asByteArray asString
	]

	display [
		self do: [:x | x print ]
	]

	includes: value [
		self do: [:x | (x = value) ifTrue: [ ^ true ] ].
		^ false
	]

	inject: thisValue into: binaryBlock [     | last |
		last <- thisValue.
		self do: [:x | last <- binaryBlock value: last value: x].
		^ last
	]

	isEmpty [
		^ self size == 0
	]

	occurrencesOf: anObject [
		^ self inject: 0
		       into: [:x :y | (y = anObject) 
					 ifTrue: [x + 1]
					 ifFalse: [x] ]
	]

	printString [
		^ ( self inject: self class printString , ' ('
			 into: [:x :y | x , ' ' , y printString]), ' )'
	]

	size [
		^ self inject: 0 into: [:x :y | x + 1]
	]

	sort [
		^ self sort: [:x :y | x < y ]
	]

	sort: aBlock [
		^ self inject: List new
			into: [:x :y | x add: y ordered: aBlock. x]
	]


]