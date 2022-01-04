nil subclass: Object [
    class>> (instancetype) new [ ^ 42 ]
]
Object subclass: Integer []
Object subclass: Symbol []

Object subclass: True [
    (False) asFalse [ ^false ]
]
Object subclass: False [
    (Integer) asFalse [ ^ 45 ]
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
        ^ trueCltn map: mapFun.
        ^ falseCltn map: mapFun
    ]
]
