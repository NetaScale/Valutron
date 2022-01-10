nil subclass: Object [
    class>> (instancetype) new [ ^ 42 ]
]

Object subclass: Character []
Object subclass: Dictionary []
Object subclass: Float []
Object subclass: Integer []
Object subclass: Process []
Object subclass: Scheduler []
Object subclass: String []
Object subclass: Symbol []
Object subclass: SymbolTable []
Object subclass: Smalltalk []

Object subclass: True [
    (False) asFalse [ ^false ]
]
Object subclass: False [
    (True) asFalse [ ^ 45 ]
]

Object subclass: Cltn<T> [
    (Cltn<N>) map: ([^N, T]) mapFun <N> [ ^ Cltn new ]
]

Object subclass: Tester [
    (True) testMeth [
        | (Cltn<True>) trueCltn
          (Cltn<False>) falseCltn
          mapFun |
        mapFun <- [ :each | each asFalse ].
        ^ trueCltn map: mapFun. "should fail with Cltn<False>"
        ^ falseCltn map: mapFun "should fail with Cltn<True>"
    ]
]
