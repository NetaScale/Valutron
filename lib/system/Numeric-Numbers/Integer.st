Number subclass: Integer
[
	class>>new [
		^ VM error: 'cannot create integers with new'
	]

    * value [		| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <68 self value>.
				  "primitive will return nil on overflow"
				  r notNil
				  	ifTrue: [ r ]
				  	ifFalse: [ self asLongInteger * value asLongInteger ]]
			ifFalse: [ super * value ]
	]

	+ value [		| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <60 self value>.
				  "primitive will return nil on overflow"
				  r notNil
				  	ifTrue: [ r ]
					ifFalse: [ self asLongInteger + value asLongInteger ]]
			ifFalse: [ super + value ]
	]

	, value [
		" used to make long integer constants "
		^ self * 1000 + value
	]

	- value [		| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <61 self value>.
				  "primitive will return nil on overflow"
				r notNil ifTrue: [ r ]
				ifFalse: [ self asLongInteger - value asLongInteger ]]
			ifFalse: [ super - value ]
	]

	/ value [		| t b |
		value = 0 ifTrue: [ ^ VM error: 'division by zero'].

		value isInteger
			ifTrue: [ b <- self gcd: value .
				  t <- self quo: b.
				  b <- value quo: b.
				  b negative
					ifTrue: [ t <- t negated.
						  b <- b negated ].
				  (b = 1) ifTrue: [ <161 'Return T'> . ^ t ].
				  ^ Fraction top: t bottom: b ]
			ifFalse: [ ^ super / value ]
	]

	< value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <62 self value> ]
			ifFalse: [ super < value ]
	]

	= value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ self == value ]
			ifFalse: [ super = value ]
	]

	> value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <63 self value> ]
			ifFalse: [ super > value ]
	]

	allMask: value [
		" see if all bits in argument are on"
		^ value = (self bitAnd: value)
	]

	anyMask: value [
		" see if any bits in argument are on"
		^ 0 ~= (self bitAnd: value)
	]

	asCharacter [
		^ Char value: self
	]

	asDigit [
		" return as character digit "
		<161 self>.
		(self >= 0)
			ifTrue: [ (self <= 9) ifTrue: 
					[ ^ (self + $0 asInteger) asCharacter ].
				  (self < 36) ifTrue:
					[ ^ (self + $A asInteger - 10) asCharacter ] ].
		^ VM error: 'illegal conversion, integer to digit'
	]

	asFloat [
		" should be redefined by any subclasses "
		self isShortInteger ifTrue: [ ^ <51 self> ]
	]

	asFraction [
		^ Fraction top: self bottom: 1
	]

	asLongInteger [	| newList i |
		newList <- List new.
		i = 0 ifTrue: [ newList add: 0 ]
			ifFalse: [ i <- self abs.
				   [ i ~= 0 ] whileTrue:
					[ newList addLast: (i rem: 100).
					i <- i quo: 100 ] ].
		^ LongInteger negative: i negative digits: newList asArray
	]

	asString [
		^ self radix: 10
	]

	bitAnd: value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <71 self value > ]
			ifFalse: [ VM error: 
				'arguments to bit operation must be short integer']
	]

	bitAt: value [
		^ (self bitShift: 1 - value) bitAnd: 1
	]

	bitInvert [
		"invert all bits in self"
		^ self bitXor: -1
	]

	bitOr: value [
		^ (self bitXor: value) bitXor: (self bitAnd: value)
	]

	bitShift: value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <79 self value > ]
			ifFalse: [ VM error: 
				'argument to bit operation must be integer']
	]

	bitXor: value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <72 self value > ]
			ifFalse: [ VM error: 
				'argument to bit operation must be integer']
	]

	even [
		^ (self rem: 2) = 0
	]

	factorial [
		^ (2 to: self) inject: 1 into: [:x :y | x * y ]
	]

	gcd: value [
		(value = 0) ifTrue: [ ^ self ].
		(self negative) ifTrue: [ ^ self negated gcd: value ].
		(value negative) ifTrue: [ ^ self gcd: value negated ].
		(value > self) ifTrue: [ ^ value gcd: self ].
		^ value gcd: (self rem: value)
	]

	generality [
		" generality value - used in mixed class arithmetic "
		^ 2
	]

	isShortInteger [
		^ true
	]

	lcm: value [
		^ (self quo: (self gcd: value)) * value
	]

	odd [
		^ (self rem: 2) ~= 0
	]

	printString [
		^ self asString
	]

	quo: value [	| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <69 self value>.
				(r isNil)
					ifTrue: [ VM error:
						'quo: or rem: with argument 0']
					ifFalse: [ r ]]
			ifFalse: [ ^ super quo: value ]
	]

	radix: base [ 	| sa text |
		" return a printed representation of self in given base"
		sa <- self abs.
		<161 sa>.
		text <- (sa \\ base) asDigit asString.
		^ (sa < base)
			ifTrue: [ (self negative)
					ifTrue: [ '-' , text ]
					ifFalse: [ text ] ]
			ifFalse: [ ((self quo: base) radix: base), text ]
	]

	timesRepeat: aBlock [	| i |
		" use while, which is optimized, not to:, which is not"
		i <- 0.
		[ i < self ] whileTrue:
			[ aBlock value. i <- i + 1]
	]

	truncated [
		^ self
	]

]