nil subclass: Object [
    class>> (instancetype) new [ ^ 42 ]
]
Object subclass: True [
    (False) asFalse [ ^false ]
]
Object subclass: False []
Object subclass: Integer []

Object subclass: Cltn<T> [
    (Cltn<N>) map: ([^N, T]) mapFun <N> [ ^ Cltn new ]
]

Object subclass: Tester [
    (True) testMeth [
        | (Cltn<True>) trueCltn
          mapFun |
        ^ trueCltn map: [ :each | each asFalse ].
    ]
]
