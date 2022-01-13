Integer subclass: LongInteger [
    | (Boolean)negative (Array)digits |

	class>>(instancetype) negative: (Boolean)nBool digits: (Array)dArray [
		^ self basicNew negative: nBool digits: dArray
	]

	class>>(instancetype) new [
		^ self basicNew
			negative: nil
			digits: nil
	]


	(id) * (Number)n [		| result |
		n isShortInteger ifTrue: [ ^ self timesShort: n ].
		n isLongInteger  ifFalse: [ ^ super * n ].
		result <- 0 asLongInteger.
		digits reverseDo:
			[:x | result <- (result timesShort: 100) +
				(n timesShort: x)].
		negative ifTrue: [ result <- result negated ].
		^ result
	]

	(id) + (Number)n [		| newDigits z carry |
		<162 'Hello, Hello'>.
		n isLongInteger
			ifFalse: [ ^ super + n ].
		negative ifTrue: [ ^ n - self negated ].
		n negative ifTrue: [ ^ self - n negated ].
		" reduced to positive + positive case "
		newDigits <- List new.  carry <- 0.
		self with: n bitDo:
			[:x :y | z <- x + y + carry.
				(z >= 100) ifTrue: [ carry <- 1. z <- z - 100]
					 ifFalse: [ carry <- 0 ].
				newDigits addLast: z ].
		carry > 0 ifTrue: [ newDigits addLast: carry ].
		^ LongInteger negative: false digits: newDigits asArray
	]

	(id) - (Number)n [	| result newDigits z borrow |
		n isLongInteger
			ifFalse: [ ^ super - n ].
		negative ifTrue: [ ^ (self negated + n) negated ].
		n negative ifTrue: [ ^ self + n negated ].
		(self < n) ifTrue: [ ^ (n - self) negated ].
		" reduced to positive - smaller positive "
		newDigits <- List new. borrow <- 0.
		self with: n bitDo:
			[:x :y | z <- (x - borrow) - y.
				(z >= 0) ifTrue: [ borrow <- 0]
				ifFalse: [ z <- z + 100. borrow <- 1].
				newDigits addLast: z ].
		result <- 0. "now normalize result by multiplication "
		newDigits reverseDo: [:x | result <- result * 100 + x ].
		^ result
	]

	(Boolean) < (Number)n [		| result |
		n isLongInteger
			ifFalse: [ ^ super < n ].
		(negative == n negative) ifFalse: [ ^ negative ].
		" now either both positive or both negative "
		result <- false.
		self with: n bitDo:
			[:x :y | (x ~= y) ifTrue: [ result <- x < y]].
		negative ifTrue: [ result <- result not ].
		^ result
	]

	(Boolean) = (Number)n [
		n isLongInteger
			ifFalse: [ ^ super = n ].
		(negative == n negative) ifFalse: [ ^ false ].
		^ digits = n digits
	]

	(LongInteger) abs [
		negative ifTrue: [ ^ self negated]
	]

	(Float) asFloat [		| r |
		r <- 0.0 .
		digits reverseDo: [ :x | r <- r * 100.0 + x asFloat].
		negative ifTrue: [ r <- r negated ].
		^ r.
	]

	(id) bitShift: (Number)n [
		(n >= 0)
			ifTrue: [ ^ self * (2 raisedTo: n) ]
			ifFalse: [ ^ self quo: (2 raisedTo: n negated)]
	]

	(LongInteger) coerce: (Number)n [
		^ n asLongInteger
	]

	(Array)digits [
		^ digits
	]

	(Integer) generality [
		^ 4 "generality value - used in mixed type arithmetic "
	]

	(Boolean) isLongInteger [
		^ true
	]

	(Boolean) isShortInteger [
		" override method in class Integer "
		^ false
	]

	(LongInteger) negated [
		^ LongInteger negative: negative not digits: digits
	]

	(Boolean) negative [
		^ negative
	]

	(self) negative: (Boolean) nBool digits: dArray [
		negative <- nBool.
		digits <- dArray
	]

	(String) printString [	| str |
		str <- negative ifTrue: [ '-' ] ifFalse: [ '' ].
		digits reverseDo: [:x | str <- str ,
			(x quo: 10) printString , (x rem: 10) printString ].
		^ str
	]

	(id) quo: (Number)value [	| a b quo result |
		result <- 0.
		a <- self abs. b <- value abs.
		[a > b] whileTrue:
			[ quo <- (a asFloat quo: b). result <- result + quo.
				a <- a - (b * quo) ].
		^ result
	]

	(self) sign: (Boolean) s digits: d [
		negative <- s.
		digits <- d.
	]

	(LongInteger) timesShort: (Number)value [	| y z carry newDigits |
		y <- value abs.
		carry <- 0.
		newDigits <- digits collect:
			[:x | z <- x * y + carry.
				carry <- z quo: 100.
				z - (carry * 100)].
		(carry > 0) ifTrue: [ newDigits <- newDigits grow: carry ].
		^ LongInteger negative: (negative xor: value negative)
					digits: newDigits
	]

	with: (Number)n bitDo: aBlock [	| d di dj |
		" run down two digits lists in parallel doing block "
		di <- digits size.
		d <- n digits.
		dj <- d size.
		(1 to: (di max: dj)) do: [:i |
			aBlock value:
			   ((i <= di) ifTrue: [ digits at: i] ifFalse: [0])
				value:
			   ((i <= dj) ifTrue: [ d at: i] ifFalse: [0]) ]
	]

]