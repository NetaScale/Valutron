Object subclass: Magnitude [

	< value [
		^ (self <= value) and: [ self ~= value ]
	]

	<= value [
		^ (self < value) or: [ self = value ]
	]

	= value [
		^ (self == value)
	]

	> value [
		^ (value < self)
	]

	>= value [
		^ value <= self
	]

	between: low and: high [
		^ (low <= self) and: [ self <= high ]
	]

	isChar [
		^ false
	]

	max: value [
		^ (self < value)
			ifTrue: [ value ]
			ifFalse: [ self ]
	]

	min: value [
		^ (self < value)
			ifTrue: [ self ]
			ifFalse: [ value ]
	]

	~= value [
		^ (self = value) not
	]


]