Number subclass: Float [
	(id) * (Number)value [
		^ value isFloat
			ifTrue: [ <#floatMultiply self value> ]
			ifFalse: [ super * value ]
	]
	(id) + (Number)value [
		^ value isFloat
			ifTrue: [ <#floatAdd self value> " floating add " ]
			ifFalse: [ super + value ]
	]
	(id) - (Number)value [
		^ value isFloat
			ifTrue: [ <#floatSubtract self value> " floating subtract " ]
			ifFalse: [ super - value ]
	]
	(id) / (Number)value [
		^ value isFloat
			ifTrue: [ (value = 0.0)
					ifTrue: [ smalltalk error:
						'float division by zero' ]
					ifFalse: [ <#floatDivide self value> ]]
			ifFalse: [ super / value ]
	]
	(Boolean) < (Number)value [
		^ value isFloat
			ifTrue: [ <#floatLessThan self value> " floating comparison " ]
			ifFalse: [ super < value ]
	]
	(Boolean) = (id)value [
		^ value isFloat
			ifTrue: [ <#floatEqual self value> ]
			ifFalse: [ super = value ]
	]
	(Float) coerce: (Number)value [
		" convert the value into a floating point number "
		^ value asFloat
	]
	(Float) exp [
		" return e raised to self "
		^ <#floatERaisedTo self>
	]
	(Integer) generality [
		" our numerical generality - used for mixed mode arithmetic"
		^ 7
	]
	(Integer) integerPart [	| i j |
		i <- <#floatIntegerPart self>. j <- i basicAt: 2. i <- i basicAt: 1.
		j < 0 ifTrue: [ ^ 0 ] ifFalse: [ ^ i * (2 raisedTo: j)]
	]
	(Boolean) isFloat [
		^ true
	]
	(Float) ln [
		" natural log of self "
		^ <#floatNaturalLog self>
	]
	(String) printString [
		^ <#floatAsString self>
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