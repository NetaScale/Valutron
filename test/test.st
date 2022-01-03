nil subclass: Object [
]

Object subclass: True [

]

Object subclass: False [

]

Object subclass: Cltn<ElType> [
    | (ElType) el (True) other |
    (ElType) getEl [
        ^ other
    ]

    (ElType) setEl: (ElType) anEl [
        el := anEl
    ]
]

Object subclass: Tester [
    | (Cltn) cltn (Object) obj |
    (Object) testMeth <T, T2> [
        ^ cltn setEl: obj.
        ^ cltn getEl
    ]
]
