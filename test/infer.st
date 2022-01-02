Object subclass: Cltn<T> [
    | (T) el |

    (InstanceType) class>>new [
    ]

    (T) getEl [
        ^ el
    ]

    putEl: (T) anEl [
        el <- anEl
    ]
]

Cltn<T1> subclass: SubCltn<T1, T2> [

]

| testCltn (Integer)testEl |
"after this it should be inferred that testCltn is a SubCltn<NotYetInferred,
 NotYetInferred>"
testCltn = Cltn new.

"after this, we can infer that that the Cltn::T property of testCltn type is
 Integer. there exists an `Identity from: Cltn::T to: SubCltn::T1`, therefore
 the type of testCltn ought to now be refined to SubCltn<Integer,
 NotYetInferred>"
testCltn putEl: testEl.
