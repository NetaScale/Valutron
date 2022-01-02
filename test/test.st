nil subclass: Object [
]

Object subclass: Cltn<ElType> [
    | (ElType) el |
    (ElType) getEl [
        ^ el
    ]
]

Object subclass: Tester [
    | (Cltn<Object>) cltn |
    (Object) testMeth <T, T2> [
        ^ cltn getElz
    ]
]
