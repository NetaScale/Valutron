Object subclass: Method [
	| text message bytecodes literals stackSize temporarySize class watch |

	display [
		('Method ', message) print.
		'text' print.
		text print.
		'literals' print.
		literals print.
		'bytecodes' print.
		bytecodes class print.
		bytecodes do: [:x |
			(x printString, ' ', (x quo: 16), ' ', (x rem: 16))
				print ]
	]

	executeWith: arguments [
		^ (Context
			method: self
			arguments: arguments
			temporaries: (Array new: temporarySize) )
		   returnToBlock: 1
	]

	message: aSymbol [
		message <- aSymbol
	]

	methodClass [
		^class
	]

	methodClass: aClass [
  		class <- aClass
	]

	name [
		^ message
	]

	printString [
		^ message asString
	]

	signature [
		^ class asString,' ', message asString
	]

	text [
		^ (text notNil)
			ifTrue: [ text ]
			ifFalse: [ 'text not saved']
	]

	text: aString [
		text <- aString
	]

	trimmedText [	| dlm ans |
		dlm <- 10 asCharacter.
		(ans <- self text) isEmpty ifTrue: [
			^ans ].
		[ (ans at: 1) == dlm ] whileTrue: [
			ans <- ans copyFrom: 2 to: ans size ].
		[ (ans at: ans size) == dlm ] whileTrue: [
			ans <- ans copyFrom: 1 to: ans size - 1 ].
		^ans
	]

	watch: aBlock [
		watch <- aBlock
	]

	watchWith: arguments [
		" note that we are being watched "
		text print.
		watch value: arguments.
		^ self executeWith: arguments
	]


]