nil subclass: Object [
    class>> (instancetype) new [ ^ 42 ]
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

Boolean subclass: True [
    (False) not [
        ^ false.
    ]
]
Boolean subclass: False [
    (True) not [
        ^ true.
    ]
]

Object subclass: Tester [
    (Boolean) testMeth [
        | (True | False) union |
        ^ union not "succeeds with False | True"
    ]
]
