ByteArray subclass: String [

	class>>basicNew: size [
		^ self primBytes: size + 1
	]

	class>>new: size [
		^ self primBytes: size + 1
	]

	class>>new: size with: aCharacter [
		| new |
		new <- self primBytes: size + 1.
		(1 to: size) do: [ :i |
			new basicAt: i put: aCharacter
		].
		^ new
	]

	, value [
		(value isMemberOf: String)
			ifTrue: [ ^ <#stringCat self value> ]
			ifFalse: [ ^ self , value asString ]
	]

	< value [
		(value isKindOf: String)
			ifTrue: [ ^ super < value ]
			ifFalse: [ ^ false ]
	]

	= value [
		(value isKindOf: String)
			ifTrue: [ ^ super = value ]
			ifFalse: [ ^ false ]
	]

	asByteArray [	| newArray i |
		newArray <- ByteArray new: self size.
		i <- 0.
		self do: [:x | i <- i + 1. newArray at: i put: x asInteger].
		^ newArray
	]

	asInteger [
		^ self inject: 0 into: [:x :y | x * 10 + y digitValue ]
	]

	asString [
		^ self
	]

	asSymbol [
		^ <#stringAsSymbol self>
	]

	basicAt: index [
		^  (super basicAt: index) asCharacter
	]

	basicAt: index put: aValue [
		(aValue isMemberOf: Character)
			ifTrue: [ super basicAt: index put: aValue asInteger ]
			ifFalse: [ VM error:
				'cannot put non Character into string' ]
	]

	copy [
		" catenation makes copy automatically "
		^ '',self
	]

	copyFrom: position1 to: position2 [
		^ <#copyFromTo self position1 position2>
	]

	"Make a copy of the string, truncating or padding to length size."
	copyTruncateOrPadTo: size with: aCharacter [
		(self size > size) ifTrue: [
			^ self copyFrom: 1 to: size
		].
		(self size = size) ifTrue: [ ^ self copy ].
		"otherwise, pad"
		^ self copyPaddingTo: size with: aCharacter.
	]

	"Make a copy of the string, padding to length size."
	copyPaddingTo: size with: aCharacter [
		| copy |
		^ self, (String new: (size - self size max: 0) with: aCharacter)
	]

	edit [	| file text |
		"(file <- File new)
			yourself;
			scratchFile;
			open: 'w';
			print: self;
			close."
		"(editor, ' ', file name) unixCommand."
		file open: 'r'.
		text <- file asString.
		file yourself; close; delete.
		^ text
	]

	execute [	| meth |
		" execute self as body of a method "
		"meth <- Parser new parse: 'DoIt ' , self in: UndefinedObject."
		^ meth notNil ifTrue: [
			meth executeWith: (Array new: 1) ]	"nil"
		ifFalse: [
			nil ]
	]

	hash [
		^ <#stringHash self>
	]

	print [
		"stdout print: self"
		^ 'fixMe'
	]

	printString [
		^ '' , self , '' "'''', ''''"
	]

	size [
		^ <#stringSize self>
	]

	trimmed [	| dlm ans |
		dlm <- 10 asCharacter.
		(ans <- self) isEmpty ifTrue: [
			^ans ].
		[ (ans at: 1) == dlm ] whileTrue: [
			ans <- ans copyFrom: 2 to: ans size.
			ans isEmpty ifTrue: [
				^ ans ] ].
		[ (ans at: ans size) == dlm ] whileTrue: [
			ans <- ans copyFrom: 1 to: ans size - 1.
			ans isEmpty ifTrue: [
				^ ans ] ].
		^ans
	]

	unixCommand [
		^ "<88 self>" 'fixme'
	]

	value [
		" evaluate self as an expression "
		^ ( '^ [ ', self, ' ] value' ) execute
	]

	words: aBlock [	| text index list |
		list <- List new.
		text <- self.
		[ text <- text copyFrom:
			(text indexOf: aBlock ifAbsent: [ text size + 1])
				to: text size.
		  text size > 0 ] whileTrue:
			[ index <- text
				indexOf: [:x | (aBlock value: x) not ]
				ifAbsent: [ text size + 1].
			  list addLast: (text copyFrom: 1 to: index - 1).
			  text <- text copyFrom: index to: text size ].
		^ list asArray
	]
]