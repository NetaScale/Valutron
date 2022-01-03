Boolean subclass: True [

class>>new [
  "there is only one true value"
  ^ true
]

not [
  ^ false
]

asString [
  ^ 'true'
]

printString [
  ^ 'true'
]

ifTrue: trueBlock ifFalse: falseBlock [
  ^ trueBlock value
]

ifFalse: falseBlock ifTrue: trueBlock [
  ^ trueBlock value
]

or: aBlock [
  ^ true
]

and: aBlock [
  ^ aBlock value
]

"|" orBool: aBool [
  ^ true
]

& aBool [
  ^ aBool
]

]