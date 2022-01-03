Magnitude subclass: Char [
	| value |

	class>>	value: aValue [
		^ self new value: aValue
	]

	< aValue [
		" can only compare characters to characters "
		^ aValue isChar
			ifTrue: [ value < aValue asInteger ]
			ifFalse: [ VM error: 'char compared to nonchar']
	]

	== aValue [
		^ aValue isChar
			ifTrue: [ value = aValue asInteger ]
			ifFalse: [ false ]
	]

	asInteger [
		^ value
	]

	asString [
		" make ourselves into a string "
		^ ' ' copy yourself; at: 1 put: self; yourself
	]

	digitValue [
		" return an integer representing our value "
		self isDigit ifTrue: [ ^ value - $0 asInteger ].
		self isUppercase ifTrue: [ ^ value - $A asInteger + 10 ].
		^ VM error: 'illegal conversion, char to digit'
	]

	isAlphaNumeric [
		" will also accept underscores (by edict of Zak) "
		^ ((self isAlphabetic) or: [ self isDigit ]) or: [ value = 95 ]
	]

	isAlphabetic [
		^ (self isLowercase) or: [ self isUppercase ]
	]

	isBlank [
		^ value = $  asInteger " blank char "
	]

	isChar [
		^ true
	]

	isDigit [
		^ value between: $0 asInteger and: $9 asInteger
	]

	isLowercase [
		^ value between: $a asInteger and: $z asInteger
	]

	isUppercase [
		^ value between: $A asInteger and: $Z asInteger
	]

	printString [
		^ '$', self asString
	]

	value: aValue [		" private - used for initialization "
		value <- aValue
	]


]