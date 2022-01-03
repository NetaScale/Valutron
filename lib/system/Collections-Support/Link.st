Object subclass: Link [
    | key value nextLink |

	class>>FNkey: aKey value: aValue [
		^ self new key: aKey value: aValue
	]

	class>>FNvalue: aValue [
		^ self new value: aValue
	]

	class>>FNvalue: aValue link: aLink [
		^ self new value: aValue link: aLink
	]


	add: newValue whenFalse: aBlock [
		(aBlock value: value value: newValue)
			ifTrue: [ (nextLink notNil)
				ifTrue: [ nextLink <- nextLink add: newValue
					whenFalse: aBlock ]
			ifFalse: [ nextLink <- Link value: newValue] ]
			ifFalse: [ ^ Link value: newValue link: self ]
	]

	at: aKey ifAbsent: exceptionBlock [
		(aKey = key)
			ifTrue: [ ^value ]
			ifFalse: [ ^ (nextLink notNil)
					ifTrue: [ nextLink at: aKey
						    ifAbsent: exceptionBlock ]
					ifFalse: exceptionBlock ]
	]

	at: aKey put: aValue [
		(aKey = key)
			ifTrue: [ value <- aValue ]
			ifFalse: [ (nextLink notNil)
				ifTrue: [ nextLink at: aKey put: aValue]
				ifFalse: [ nextLink <- Link
						key: aKey value: aValue] ]
	]

	binaryDo: aBlock [
		aBlock value: key value: value.
		(nextLink notNil)
			ifTrue: [ nextLink binaryDo: aBlock ]
	]

	includesKey: aKey [
		(key = aKey)
			ifTrue: [ ^ true ].
		(nextLink notNil)
			ifTrue: [ ^ nextLink includesKey: aKey ]
			ifFalse: [ ^ false ]
	]

	key [
		^ key
	]

	key: aKey [
		key <- aKey
	]

	key: aKey value: aValue [
		key <- aKey.
		value <- aValue
	]

	link: aLink [
		nextLink <- aLink
	]

	next [
		^ nextLink
	]

	removeKey: aKey [
		(aKey = key)
			ifTrue: [ ^ nextLink ]
			ifFalse: [ (nextLink notNil)
				ifTrue: [ nextLink <- nextLink removeKey: aKey]]
	]

	removeValue: aValue [
		(aValue = value)
			ifTrue: [ ^ nextLink ]
			ifFalse: [ (nextLink notNil)
				ifTrue: [ nextLink <- nextLink removeValue: aValue]]
	]

	reverseDo: aBlock [
		(nextLink notNil)
			ifTrue: [ nextLink reverseDo: aBlock ].
		aBlock value: value
	]

	size [
		(nextLink notNil)
			ifTrue: [ ^ 1 + nextLink size]
			ifFalse: [ ^ 1 ]
	]

	value [
		^ value
	]

	value: aValue [
		value <- aValue
	]

	value: aValue link: aLink [
		value <- aValue.
		nextLink <- aLink
	]

]