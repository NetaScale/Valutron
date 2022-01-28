Link subclass: ValueLink<T> [
	| (T) value |

	class>>(instancetype)value: value [
		^ super new value: value
	]

	(T)value [
		^ value
	]

	(self)value: (T)anObject [
		value <- anObject
	]
]