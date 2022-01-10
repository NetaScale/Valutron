nil subclass: Object [
    class>> (instancetype) new [ ^ 55 ]
]

Object subclass: Character []
Object subclass: Dictionary []
Object subclass: Float []
Object subclass: True []
Object subclass: False []
Object subclass: Integer []
Object subclass: Process []
Object subclass: Scheduler []
Object subclass: String []
Object subclass: Symbol []
Object subclass: SymbolTable []
Object subclass: Smalltalk []

Object subclass: Identity [
    (T) identity: (T) anObj <T> [
        ^ anObj
    ]
]

Object subclass: Tester [
    (True) testMeth [
        | ident |
        ident <- Identity new.
        ^ ident identity: 55. "fails with Integer"
        ^ ident identity: #Sym "fails with Symbol"
    ]
]
