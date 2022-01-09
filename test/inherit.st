nil subclass: Object [
    class>>(instancetype) new [
        ^ 55
    ]
]
Object subclass: True []
Object subclass: False []
Object subclass: Integer []
Object subclass: String []
Object subclass: Symbol []

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
        cltn setEl: 55.
	^ cltn
    ]
]
