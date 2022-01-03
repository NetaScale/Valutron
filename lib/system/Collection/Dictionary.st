IndexedCollection subclass: Dictionary [
    | hashTable |

    class>>new [
		^ self basicNew
			hashTable: (Array new: 39)
	]

	at: aKey ifAbsent: exceptionBlock [	| hashPosition  link |
		hashPosition <- self hash: aKey.
		((hashTable at: hashPosition + 1) = aKey)
			ifTrue: [ ^ hashTable at: hashPosition + 2].
		link <- hashTable at: hashPosition + 3.
		^ (link notNil)
			ifTrue: [ link at: aKey ifAbsent: exceptionBlock ]
			ifFalse: exceptionBlock
	]

	at: aKey put: aValue [			| hashPosition link |
		hashPosition <- self hash: aKey.
		((hashTable at: hashPosition + 1) isNil)
		   ifTrue: [ hashTable at: hashPosition + 1 put: aKey ].
		((hashTable at: hashPosition + 1) = aKey)
		   ifTrue: [ hashTable at: hashPosition + 2 put: aValue ]
		   ifFalse: [ link <- hashTable at: hashPosition + 3.
			(link notNil)
				ifTrue: [ link at: aKey put: aValue ]
				ifFalse: [ hashTable at: hashPosition + 3
					put: (Link key: aKey value: aValue)]]
	]

	basicRemoveKey: aKey [		| hashPosition link |
		hashPosition <- self hash: aKey.
		((hashTable at: hashPosition + 1) = aKey)
			ifTrue: [ link <- hashTable at: hashPosition + 3.
				(link notNil) ifTrue: [
					hashTable at: hashPosition + 1 put: link key.
					hashTable at: hashPosition + 2 put: link value.
					hashTable at: hashPosition + 3 put: link next ]
				ifFalse: [
					hashTable at: hashPosition + 1 put: nil.
					hashTable at: hashPosition + 2 put: nil ] ]
			ifFalse: [ link <- hashTable at: hashPosition + 3.
				(link notNil) ifTrue: [
					hashTable
						at: hashPosition + 3
						put: (link removeKey: aKey) ] ]
	]

	binaryDo: aBlock [
		(1 to: hashTable size by: 3) do:
			[:i | (hashTable at: i) notNil
				ifTrue: [ aBlock value: (hashTable at: i)
						value: (hashTable at: i+1) ].
			      (hashTable at: i+2) notNil
				ifTrue: [ (hashTable at: i+2) 
						binaryDo: aBlock ] ]
	]

	display [
		self binaryDo: [:x :y | (x printString , ' -> ', 
					y printString ) print ]
	]

	hash: aKey [
		^ 3 * ((aKey hash) rem: ((hashTable size) quo: 3))
	]

	hashTable: hArray [
		hashTable <- hArray
	]

	includesKey: aKey [
		" look up, but throw away result "
		self at: aKey ifAbsent: [ ^ false ].
		^ true
	]

	removeKey: aKey [
		^ self removeKey: aKey
			ifAbsent: [ VM error: 'remove key not found']
	]

	removeKey: aKey ifAbsent: exceptionBlock [
		^ (self includesKey: aKey)
			ifTrue: [ self basicRemoveKey: aKey ]
			ifFalse: exceptionBlock
	]

]

Dictionary subclass: SymbolTable [
    hash: aKey [
		^ 3 * ((aKey hash) rem: ((hashTable size) quo: 3))
    ]

	printString [
		^ self class printString , ' (...)'
    ]
]

SymbolTable subclass: SystemDictionary [
	at: key [
		<161 key>.
		^ super at: key ifAbsent: [ <163 key> ]
	]
]