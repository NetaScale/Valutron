Boolean subclass: False [
	class>>new [
	"there is only one false value"
	^ false
	]

	not [
	^ true
	]

	asString [
	^ 'false'
	]

	printString [
	^ 'false'
	]

	ifTrue: trueBlock ifFalse: falseBlock [
	^ falseBlock value
	]

	ifFalse: falseBlock ifTrue: trueBlock [
	^ falseBlock value
	]

	or: aBlock [
	^ aBlock value
	]

	and: aBlock [
	^ false
	]

	"|" orBool: aBool [
	^ aBool
	]

	& aBool [
	^ false
	]

]