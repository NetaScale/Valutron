nil subclass: Object [
    class>>(instancetype) new [
        ^ 55
    ]
]
Object subclass: True []
Object subclass: False []
Object subclass: Integer []
Object subclass: Symbol []

Object subclass: Cltn<ElType> [
    | (ElType) el (True) other |
    (ElType) getEl [
        ^ other
    ]

    (ElType) setEl: (ElType) anEl [
        el := anEl
    ]
]

Cltn<SubElType> subclass: SubCltn<SubElType> [

]

Object subclass: Tester [
    (Cltn<Integer>) testMeth <T, T2> [
        | cltn
          obj |
        cltn <- SubCltn new.
        obj <- 55.
        cltn setEl: obj.
        cltn getEl.
        ^ cltn
    ]
]
