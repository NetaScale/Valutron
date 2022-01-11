nil subclass: Object [
	class>>(instancetype) new [
		^ 55
	]

	(Boolean) isKindOf: (id)aClass [
		^ true.
	]
]

Object subclass: Character [
	(Integer) asInteger [
		^ 42
	]
]

Object subclass: Boolean [
	(A | B) ifTrue: ([^A])trueBlock ifFalse: ([^B])falseBlock <A, B> [
		^ 1
	]
]
Object subclass: Dictionary []
Object subclass: Float []
Boolean subclass: True []
Boolean subclass: False []
Object subclass: Integer []
Object subclass: Process []
Object subclass: Scheduler []
Object subclass: String []
Object subclass: Symbol []
Object subclass: SymbolTable []
Object subclass: Smalltalk []

Object subclass: Tester [
	(Float) testWith: (Object)var [
		"^ (var isKindOf: Character or: [ var isKindOf: Boolean ])"
		^ (var isKindOf: Character)
			ifTrue: [ var asInteger ] "should fail with Integer"
			ifFalse: [ #no ] "should fail with Symbol"
	]
]
