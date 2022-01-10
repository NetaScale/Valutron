Number subclass: Float [
	(id) * (Number)value [
		^ value isFloat
			ifTrue: [ <118 self value> ]
			ifFalse: [ super * value ]
	]
	(id) + (Number)value [
		^ value isFloat
			ifTrue: [ <110 self value> " floating add " ]
			ifFalse: [ super + value ]
	]
	(id) - (Number)value [
		^ value isFloat
			ifTrue: [ <111 self value> " floating subtract " ]
			ifFalse: [ super - value ]
	]
	(id) / (Number)value [
		^ value isFloat
			ifTrue: [ (value = 0.0)
					ifTrue: [ smalltalk error:
						'float division by zero' ]
					ifFalse: [ <119 self value> ]]
			ifFalse: [ super / value ]
	]
	(Boolean) < (Number)value [
		^ value isFloat
			ifTrue: [ <112 self value> " floating comparison " ]
			ifFalse: [ super < value ]
	]
	(Boolean) = (id)value [
		^ value isFloat
			ifTrue: [ <116 self value> ]
			ifFalse: [ super = value ]
	]
	(Float) coerce: (Number)value [
		" convert the value into a floating point number "
		^ value asFloat
	]
	(Float) exp [
		" return e raised to self "
		^ <103 self>
	]
	(Integer) generality [
		" our numerical generality - used for mixed mode arithmetic"
		^ 7
	]
	(Integer) integerPart [	| i j |
		i <- <106 self>. j <- i basicAt: 2. i <- i basicAt: 1.
		j < 0 ifTrue: [ ^ 0 ] ifFalse: [ ^ i * (2 raisedTo: j)]
	]
	(Boolean) isFloat [
		^ true
	]
	(Float) ln [
		" natural log of self "
		^ <102 self>
	]
	(String) printString [
		^ <101 self>
	]
	(id) quo: (Number)value [
		^ (self / value) truncated
	]
	(Float) rounded [
		^ (self + 0.5) floor
	]
	(Integer) truncated [	| result f i |
		" truncate to an integer rounded towards zero"
		f <- self.
		result <- 0.
		[ i <- f integerPart. i > 0] whileTrue:
			[ result <- result + i. f <- f - i ].
		^ result
	]
]