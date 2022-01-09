nil subclass: Object [
    class>> (instancetype) new [ ^ 42 ]
]
Object subclass: Integer []
Object subclass: Symbol []
Object subclass: String []
Object subclass: Boolean []

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
        ^ union not
    ]
]
