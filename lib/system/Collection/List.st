Collection subclass: List [
    | links |

	add: aValue [
		^ self addLast: aValue
	]

	add: aValue ordered: aBlock [
		(links isNil)
			ifTrue: [ self addFirst: aValue]
			ifFalse: [ links <- links add: aValue 
					whenFalse: aBlock ]
	]

	addAll: aValue [
		aValue do: [:x | self add: x ]
	]

	addFirst: aValue [
		links <- Link value: aValue link: links
	]

	addLast: aValue [
		(links isNil)
			ifTrue: [ self addFirst: aValue ]
			ifFalse: [ links add: aValue whenFalse: [ :x :y | true ] ]
	]

	collect: aBlock [
		^ self inject: self class new
		       into: [:x :y | x add: (aBlock value: y). x ]
	]

	do: aBlock [
		(links notNil)
			ifTrue: [ links binaryDo: [:x :y | aBlock value: y]]
	]

	first [
		^ (links notNil)
			ifTrue: links
			ifFalse: [ smalltalk error: 'first on empty list']
	]

	links [
		^ links  "used to walk two lists in parallel "
	]

	reject: aBlock [
		^ self select: [:x | (aBlock value: x) not ]
	]

	remove: value [
		(links notNil)
			ifTrue: [ links <- links removeValue: value ]
	]

	removeFirst [
		self remove: self first
	]

	reverseDo: aBlock [
		(links notNil)
			ifTrue: [ links reverseDo: aBlock ]
	]

	select: aBlock [
		^ self inject: self class new
		       into: [:x :y | (aBlock value: y) 
					ifTrue: [x add: y]. x]
	]

	size [
		(links isNil)
			ifTrue: [ ^ 0 ]
			ifFalse: [ ^ links size ]
	]

]