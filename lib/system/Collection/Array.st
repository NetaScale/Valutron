IndexedCollection subclass: Array [

    class>>basicNew [
		^ self basicNew: 0
	]

	class>>new [
		^ self new: 0
	]


	< coll [
		(coll isKindOf: Array)
			ifTrue: [ self with: coll 
				   do: [:x :y | (x = y) ifFalse: 
						  [ ^ x < y ]].
				  ^ self size < coll size ]
			ifFalse: [ ^ super < coll ]
	]

	= coll [
		(coll isKindOf: Array)
			ifTrue: [ (self size = coll size)
					ifFalse: [ ^ false ].
				  self with: coll
					do: [:x :y | (x = y) 
						ifFalse: [ ^ false ] ]. 
				 ^ true ]
			ifFalse: [ ^ super = coll ]
	]

	at: index put: value [
		(self includesKey: index)
			ifTrue: [ self basicAt: index put: value ]
			ifFalse: [ VM error: 
				'illegal index to at:put: for array' ]
	]

	binaryDo: aBlock [
		(1 to: self size) do:
			[:i | aBlock value: i value: (self at: i) ]
	]

	collect: aBlock [		| s newArray |
		s <- self size.
		newArray <- Array new: s.
		(1 to: s) do: [:i | newArray at: i put: 
			(aBlock value: (self at: i))].
		^ newArray
	]

	copyFrom: low to: high [	| newArray newlow newhigh |
		newlow <- low max: 1.
		newhigh <- high min: self size.
		newArray <- self class new: (0 max: newhigh - newlow + 1).
		(newlow to: newhigh)
			do: [:i |  newArray at: ((i - newlow) + 1)
					put: (self at: i) ].
		^ newArray
	]

	deepCopy [
		^ self deepCopyFrom: 1 to: self size
	]

	deepCopyFrom: low to: high [	| newArray newlow newhigh |
		newlow <- low max: 1.
		newhigh <- high min: self size.
		newArray <- self class new: (0 max: newhigh - newlow + 1).
		(newlow to: newhigh)
			do: [:i |  newArray at: ((i - newlow) + 1)
					put: (self at: i) copy ].
		^ newArray
	]

	do: aBlock [
		(1 to: self size) do:
			[:i | aBlock value: (self at: i) ]
	]

	exchange: a and: b [	| temp |
		temp <- self at: a.
		self at: a put: (self at: b).
		self at: b put: temp
	]

	grow: aValue [	| s newArray |
		s <- self size.
		newArray <- Array new: s + 1.
		(1 to: s) do: [:i | newArray at: i put: (self at: i)].
		newArray at: s+1 put: aValue.
		^ newArray
	]

	includesKey: index [
		^ index between: 1 and: self size
	]

	reverseDo: aBlock [
		(self size to: 1 by: -1) do:
			[:i | aBlock value: (self at: i) ]
	]

	select: aCond [	| newList |
		newList <- List new.
		self do: [:i | (aCond value: i) ifTrue: [newList addLast: i]].
		^ newList asArray
	]

	shallowCopy [
		^ self copyFrom: 1 to: self size
	]

	size [
		^ self basicSize
	]

	with: newElement [	| s newArray |
		s <- self size.
		newArray <- Array new: (s + 1).
		(1 to: s) do: [:i | newArray at: i put: (self at: i) ].
		newArray at: s+1 put: newElement.
		^ newArray
	]

	with: coll do: aBlock [
		(1 to: (self size min: coll size))
			do: [:i | aBlock value: (self at: i) 
					value: (coll at: i) ]
	]

	with: coll ifAbsent: z do: aBlock [	| xsize ysize |
		xsize <- self size.
		ysize <- coll size.
		(1 to: (xsize max: ysize))
			do: [:i | aBlock value:
			  (i <= xsize ifTrue: [ self at: i ] ifFalse: [ z ])
			  value:
			  (i <= ysize ifTrue: [ coll at: i ] ifFalse: [ z ])]
	]


]