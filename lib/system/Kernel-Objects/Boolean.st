Object subclass: Boolean [

and: aBlock [
  ^ self
    ifTrue: [ aBlock value ]
    ifFalse: [ false ]
]

or: aBlock [
  ^ self ifTrue: [ true ] ifFalse: [ aBlock value ]
]

"|" orBool: aBool [
  ^ self ifTrue: [ true ] ifFalse: [ aBool ]
]

& aBool [
  ^ self ifTrue: [ aBool ] ifFalse: [ false ]
]

not [
  ^ self ifTrue: [ false ] ifFalse: [ true ]
]

(A | B) ifFalse: ([^A])falseBlock ifTrue: ([^B])trueBlock <A, B> [
  ^ self ifTrue: [ trueBlock  value ] ifFalse: [ falseBlock value ]
]

(A | B) ifTrue: ([^A])trueBlock ifFalse: ([^B])falseBlock <A, B> [
  ^ self ifTrue: [ trueBlock  value ] ifFalse: [ falseBlock value ]
]

ifTrue: aBlock [
  ^ self ifTrue: [ aBlock value ] ifFalse: [ false ]
]

(A) ifFalse: ([^A])aBlock <A> [
  ^ self ifTrue: [ true ] ifFalse: [ aBlock value ]
]


]