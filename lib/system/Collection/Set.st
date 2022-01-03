List subclass: Set [
	add: value [
		(self includes: value)
			ifFalse: [ self addFirst: value ]
    ]
]