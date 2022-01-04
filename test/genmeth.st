nil subclass: Object [
    class>> (instancetype) new [ ^ 55 ]
]
Object subclass: True []
Object subclass: False []
Object subclass: Integer []
Object subclass: Symbol []

Object subclass: Identity [
    (T) identity: (T) anObj <T> [
        ^ anObj
    ]
]

Object subclass: Tester [
    (True) testMeth [
        | ident |
        ident <- Identity new.
        ^ ident identity: 55.
        ^ ident identity: #Sym
    ]
]
