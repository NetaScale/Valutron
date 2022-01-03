nil subclass: Object [
]

Object subclass: Object2 [

]

Object subclass: Cltn<ElType> [
    | (ElType) el |
    (ElType) getEl [
        ^ el
    ]

    (ElType) setEl: (ElType) anEl [
        el := anEl
    ]
]

Object subclass: Tester [
    | (Cltn) cltn (Object) obj |
    (Object) testMeth <T, T2> [
        ^ cltn setEl: obj
        ^ cltn getEl
    ]
]
