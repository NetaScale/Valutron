nil subclass: Object [
    class>> (instancetype) new [ ^ 55 ]
]
Object subclass: True []
Object subclass: False []
Object subclass: Integer []

"type of block should be <Arg1, RType> [^RType, Arg1]"

Object subclass: Tester [
    (True) testMeth [
        | ([^Object, Integer]) block |
        block := [ :a | a ].
        ^ block
    ]
]
