Number subclass: Float [
	* value [
		^ value isFloat
			ifTrue: [ <118 self value> ]
			ifFalse: [ super * value ]
	]
	+ value [
		^ value isFloat
			ifTrue: [ <110 self value> " floating add " ]
			ifFalse: [ super + value ]
	]
	- value [
		^ value isFloat
			ifTrue: [ <111 self value> " floating subtract " ]
			ifFalse: [ super - value ]
	]
	/ value [
		^ value isFloat
			ifTrue: [ (value = 0.0)
					ifTrue: [ smalltalk error:
						'float division by zero' ]
					ifFalse: [ <119 self value> ]]
			ifFalse: [ super / value ]
	]
	< value [
		^ value isFloat
			ifTrue: [ <112 self value> " floating comparison " ]
			ifFalse: [ super < value ]
	]
	= value [
		^ value isFloat
			ifTrue: [ <116 self value> ]
			ifFalse: [ super = value ]
	]
	coerce: value [
		" convert the value into a floating point number "
		^ value asFloat
	]
	exp [
		" return e raised to self "
		^ <103 self>
	]
	generality [
		" our numerical generality - used for mixed mode arithmetic"
		^ 7
	]
	integerPart [	| i j |
		i <- <106 self>. j <- i basicAt: 2. i <- i basicAt: 1.
		j < 0 ifTrue: [ ^ 0 ] ifFalse: [ ^ i * (2 raisedTo: j)]
	]
	isFloat [
		^ true
	]
	ln [
		" natural log of self "
		^ <102 self>
	]
	printString [
		^ <101 self>
	]
	quo: value [
		^ (self / value) truncated
	]
	rounded [
		^ (self + 0.5) floor
	]
	truncated [	| result f i |
		" truncate to an integer rounded towards zero"
		f <- self. result <- 0.
		[ i <- f integerPart. i > 0] whileTrue:
			[ result <- result + i. f <- f - i ].
		^ result
	]
]