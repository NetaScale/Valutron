nil subclass: Object [
    class>>(instancetype) new [
        ^ 55
    ]
]

Object subclass: Character []
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

Object subclass: Cltn<ElType> [
    | (ElType) el  |
    (ElType) getEl [
        ^ el
    ]

]

Object subclass: Assoc<A, B> [

]

Cltn<Assoc<A, B>> subclass: SubCltn<A, B> [
]

Object subclass: Tester [
    (True) testMeth [
        | (SubCltn<Integer, String>) cltn
	  test |
        ^ cltn getEl
    ]
]
