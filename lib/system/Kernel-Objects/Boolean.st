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

ifFalse: falseBlock ifTrue: trueBlock [
  ^ self ifTrue: [ trueBlock  value ] ifFalse: [ falseBlock value ]
]

ifTrue: trueBlock ifFalse: falseBlock [
  ^ self ifTrue: [ trueBlock  value ] ifFalse: [ falseBlock value ]
]

ifTrue: aBlock [
  ^ self ifTrue: [ aBlock value ] ifFalse: [ false ]
]

ifFalse: aBlock [
  ^ self ifTrue: [ true ] ifFalse: [ aBlock value ]
]


]