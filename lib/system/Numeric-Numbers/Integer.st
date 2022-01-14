Number subclass: Integer
[
	class>>new [
		^ VM error: 'cannot create integers with new'
	]

	(id) * (Number)value [		| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <#smiMul self value>.
				  "primitive will return nil on overflow"
				  r notNil
				  	ifTrue: [ r ]
				  	ifFalse: [ self asLongInteger * value asLongInteger ]]
			ifFalse: [ super * value ]
	]

	(id) + (Number)value [		| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <#smiAdd self value>.
				  "primitive will return nil on overflow"
				  r notNil
				  	ifTrue: [ r ]
					ifFalse: [ self asLongInteger + value asLongInteger ]]
			ifFalse: [ super + value ]
	]

	(id) , (Number)value [
		" used to make long integer constants "
		^ self * 1000 + value
	]

	(id) - (Number)value [		| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <#smiSub self value>.
				  "primitive will return nil on overflow"
				r notNil ifTrue: [ r ]
				ifFalse: [ self asLongInteger - value asLongInteger ]]
			ifFalse: [ super - value ]
	]

	(id) / (Number)value [		| t b |
		value = 0 ifTrue: [ ^ VM error: 'division by zero'].

		value isInteger
			ifTrue: [ b <- self gcd: value .
				  t <- self quo: b.
				  b <- value quo: b.
				  b negative
					ifTrue: [ t <- t negated.
						  b <- b negated ].
				  (b = 1) ifTrue: [ ^ t ].
				  ^ Fraction top: t bottom: b ]
			ifFalse: [ ^ super / value ]
	]

	(Boolean) < (Number)value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <#smi< self value> ]
			ifFalse: [ super < value ]
	]

	(Boolean) = (id)value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ self == value ]
			ifFalse: [ super = value ]
	]

	(Boolean) > (Number)value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <#smi> self value> ]
			ifFalse: [ super > value ]
	]

	(Boolean) allMask: (Integer)value [
		" see if all bits in argument are on"
		^ value = (self bitAnd: value)
	]

	(Boolean) anyMask: (Integer)value [
		" see if any bits in argument are on"
		^ 0 ~= (self bitAnd: value)
	]

	(Character) asCharacter [
		^ Character value: self
	]

	(Character) asDigit [
		" return as character digit "
		(self >= 0)
			ifTrue: [ (self <= 9) ifTrue:
					[ ^ (self + $0 asInteger) asCharacter ].
				  (self < 36) ifTrue:
					[ ^ (self + $A asInteger - 10) asCharacter ] ].
		^ VM error: 'illegal conversion, integer to digit'
	]

	(Float) asFloat [
		" should be redefined by any subclasses "
		self isShortInteger ifTrue: [ ^ <#asFloat self> ]
	]

	(Fraction) asFraction [
		^ Fraction top: self bottom: 1
	]

	(LongInteger) asLongInteger [	| newList i |
		newList <- List new.
		i = 0 ifTrue: [ newList add: 0 ]
			ifFalse: [ i <- self abs.
				   [ i ~= 0 ] whileTrue:
					[ newList addLast: (i rem: 100).
					i <- i quo: 100 ] ].
		^ LongInteger negative: i negative digits: newList asArray
	]

	(String) asString [
		^ self radix: 10
	]

	(Boolean) bitAnd: (Integer)value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <#smiBitAnd self value > ]
			ifFalse: [ VM error:
				'arguments to bit operation must be short integer']
	]

	(Boolean) bitAt: (Integer)value [
		^ (self bitShift: 1 - value) bitAnd: 1
	]

	(Integer) bitInvert [
		"invert all bits in self"
		^ self bitXor: -1
	]

	bitOr: (Integer)value [
		^ (self bitXor: value) bitXor: (self bitAnd: value)
	]

	(Integer) bitShift: (Integer)value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <#smiBitShift self value > ]
			ifFalse: [ VM error:
				'argument to bit operation must be integer']
	]

	(Integer) bitXor: (Integer)value [
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ <#smiBitXor self value > ]
			ifFalse: [ VM error:
				'argument to bit operation must be integer']
	]

	(Boolean) even [
		^ (self rem: 2) = 0
	]

	(id) factorial [
		^ (2 asLongInteger to: self) inject: 1 asLongInteger into: [:x :y | x * y ]
	]

	(id) gcd: value [
		(value = 0) ifTrue: [ ^ self ].
		(self negative) ifTrue: [ ^ self negated gcd: value ].
		(value negative) ifTrue: [ ^ self gcd: value negated ].
		(value > self) ifTrue: [ ^ value gcd: self ].
		^ value gcd: (self rem: value)
	]

	(Integer) generality [
		" generality value - used in mixed class arithmetic "
		^ 2
	]

	(Boolean) isShortInteger [
		^ true
	]

	(id) lcm: (Number)value [
		^ (self quo: (self gcd: value)) * value
	]

	(Boolean) odd [
		^ (self rem: 2) ~= 0
	]

	(String) printString [
		^ self asString
	]

	(id) quo: (Number)value [	| r |
		^ (self isShortInteger and: [value isShortInteger])
			ifTrue: [ r <- <#smiQuo self value>.
				(r isNil)
					ifTrue: [ VM error:
						'quo: or rem: with argument 0']
					ifFalse: [ r ]]
			ifFalse: [ ^ super quo: value ]
	]

	(String) radix: (Integer)base [ 	| sa text |
		" return a printed representation of self in given base"
		sa <- self abs.
		text <- (sa \\ base) asDigit asString.
		^ (sa < base)
			ifTrue: [ (self negative)
					ifTrue: [ '-' , text ]
					ifFalse: [ text ] ]
			ifFalse: [ ((self quo: base) radix: base), text ]
	]

	(self) timesRepeat: ([^id]) aBlock [	| i |
		" use while, which is optimized, not to:, which is not"
		i <- 0.
		[ i < self ] whileTrue:
			[ aBlock value. i <- i + 1]
	]

	(self) truncated [
		^ self
	]

]