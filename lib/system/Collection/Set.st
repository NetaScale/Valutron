List<T> subclass: Set<T> [
	add: value [
		(self includes: value)
			ifFalse: [ self addFirst: value ]
    ]
]