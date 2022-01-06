"higher-kinded types
 not suppported yet"

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
    | (ElType) el |
    (ElType) getEl [
        ^ el
    ]

    (ElType) setEl: (ElType) anEl [
        el := anEl
    ]
]

Object subclass: Holder<CltnType> [
    (CltnType<Integer>) get [
        ^55.
    ]
]

Object subclass: Tester [
    (Object) testMeth [
        | (Holder<Cltn>) holder
          cltn |
        cltn <- holder get.
        ^cltn
    ]
]
