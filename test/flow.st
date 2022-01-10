nil subclass: Object [
	class>>(instancetype) new [
		^ 55
	]
]

Object subclass: Character [
	(Integer) asInteger [
		^ 42
	]
]
Object subclass: Dictionary []
Object subclass: Float []
Object subclass: True []
Object subclass: False []
Object subclass: Integer []
Object subclass: Process []
Object subclass: Scheduler []
Object subclass: String []
Object subclass: Symbol []
Object subclass: SymbolTable []
Object subclass: Smalltalk []


Object subclass: Tester [
	(Float) testWith: (id)var [
		^ (var isKindOf: Character)
			ifTrue: [ var asInteger ]
			ifFalse: [ 42 ]
	]
]
