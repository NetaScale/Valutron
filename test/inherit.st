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

Cltn<SubElType> subclass: SubCltn<SubElType> [
	(SubElType) setEl: (SubElType)anEl [
		^ el <- anEl
	]
]

Object subclass: Tester [
    (True) testMeth [
        | cltn |
        cltn <- SubCltn new.
        cltn setEl: 55. "should infer ElType to be Integer"
	^ cltn "should fail with SubCltn<Integer>"
    ]
]
