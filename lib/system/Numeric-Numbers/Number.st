Magnitude subclass: Number
[
	(id) * (Number)value [
		^ (self maxgen: value) * (value maxgen: self)
	]

	(id) + (Number)value [
		^ (self maxgen: value) + (value maxgen: self)
	]

	(id) - (Number)value [
		^ (self maxgen: value) - (value maxgen: self)
	]

	(id) / (Number)value [
		^ (self maxgen: value) / (value maxgen: self)
	]

	(id) // (Number)value [
		" integer division, truncate towards negative infinity"
		" see quo: "
		^ (self / value) floor
	]

	(Boolean) < (Number)value [
		^ (self maxgen: value) < (value maxgen: self)
	]

	(Boolean) = (id)value [
		^ value isNumber
			ifTrue: [ (self maxgen: value) = (value maxgen: self) ]
			ifFalse: [ false ]
	]

	(id) \\ value [
		" remainder after integer division "
		^ self - ((self // value) * value)
	]

	(id) abs [
		^ (self < 0)
			ifTrue: [ 0 - self ]
			ifFalse: [ self ]
	]

	(id) ceiling [		| i |
		i <- self truncated.
		^ ((self positive) and: [ self ~= i ])
			ifTrue: [ i + 1 ]
			ifFalse: [ i ]
	]

	(instancetype) copy [
		^ self
	]

	(Float) exp [
		^ self asFloat exp
	]

	(id) fib [
		^ self > 1
			ifTrue: [ (self - 1) fib + (self - 2) fib ]
			ifFalse: [ self ]
	]

	(id) floor [		| i |
		i <- self truncated.
		^ ((self negative) and: [ self ~= i ])
			ifTrue: [ i - 1 ]
			ifFalse: [ i ]
	]

	(id) fractionalPart [
		^ self - self truncated
	]

	(Boolean) isInteger [
		^ self isLongInteger or: [ self isShortInteger ]
	]

	(Boolean) isNumber [
		^ true
	]

	"Natural logarithm of the number"
	(Float) ln [
		^ self asFloat ln
	]

	"Logarithm to the base of value"
	(Float) log: (Number)value [
		^ self ln / value ln
	]

	"Maximum generality number of self vs another number"
	(Number) maxgen: (Number)value [
		(self isNumber and: [ value isNumber ])
			ifFalse: [ ^ VM error:
				'arithmetic on non-numbers' ].
		^ (self generality > value generality)
			ifTrue: [ self ]
			ifFalse: [ value coerce: self ]
	]

	(id) negated [
		^ 0 - self
	]

	(Boolean) negative [
		^ self < 0
	]

	(Boolean) positive [
		^ self >= 0
	]

	(id) quo: (Number)value [
		^ (self maxgen: value) quo: (value maxgen: self)
	]

	(id) raisedTo: (Number)x [	| y |
		x negative
			ifTrue: [ ^ 1 / (self raisedTo: x negated) ].
		x isShortInteger
			ifTrue: [ (x = 0) ifTrue: [ ^ 1 ].
				  y <- (self raisedTo: (x quo: 2)) squared.
				  x odd ifTrue: [ y <- y * self ].
				  ^ y ]
				"use logrithms to do exponeneation"
			ifFalse: [ ^ ( x * self ln ) exp ]
	]

	(id) reciprocal [
		^ 1 / self
	]

	(id) rem: (Number)value [
		^ self - ((self quo: value) * value)
	]

	(id) roundTo: (Number)value [
		^ (self / value ) rounded * value
	]

	(id) sign [
		^ (self = 0) ifTrue: [ 0 ]
			ifFalse: [ self / self abs ]
	]

	(id) sqrt [
		^ (self negative)
			ifTrue: [ VM error: 'sqrt of negative']
			ifFalse: [ self raisedTo: 0.5 ]
	]

	(id) squared [
		^ self * self
	]

	(id) strictlyPositive [
		^ self > 0
	]

	(Interval) to: (Number)value [
		^ Interval lower: self upper: value step: 1
	]

	(Interval) to: (Number)value by: (Number)step [
		^ Interval lower: self upper: value step: step
	]

	(id) truncateTo: (Number)value [
		^ (self / value) trucated * value
	]

]