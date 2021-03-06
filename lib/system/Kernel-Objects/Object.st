nil subclass: Object [
	| |
	| name superClass methods nstSize nstVars |

	"Behaviour"
	class>>addMethod [	| m |
		(m <- self doEdit: '') notNil ifTrue: [
			self install: m ]
	]

	class>> (instancetype) basicNew [
		^ self primOrefs: (self basicAt: 4)
	]

	class>> (instancetype) basicNew: size [
		^ self primOrefs: size
	]

	class>> (self) display [
		('Class name: ', (self basicAt: 1) asString)  print.
		((self basicAt: 2) notNil)
			ifTrue: [ ('Superclass: ', (self basicAt: 2) ) print ].
		'Instance Variables:' print.
		(self basicAt: 5) isNil
			ifTrue: [ 'no instance variables ' print ]
			ifFalse: [ (self basicAt: 5) display ].
		'Subclasses: ' print.
		self subClasses display
	]

	class>>doEdit: aString [	| tmp ans |
		" edit a method definition until it compiles correctly "
		tmp <- aString.
		[ tmp <- tmp edit trimmed.
		  "ans <- Parser new parse: tmp in: self."
		  ans notNil ifTrue: [
			^ ans ]
		  ifFalse: [
			smalltalk inquire: 'edit again (yn) ? ' ] ] whileTrue.
		^ nil
	]

	class>>editMethod: name [	| m |
		m <- self methodNamed: name.
		m notNil ifTrue: [
			(m <- self doEdit: m text) notNil ifTrue: [
				self install: m ] ]
		ifFalse: [
			'no such method' print ]
	]

	class>>fileOut: aSym [	| aMth cStr mStr aStr aFile |
		" file out one method on class.method.st "
		(aMth <- self methodNamed: aSym) isNil ifTrue: [
			^ self ].
		cStr <- aMth methodClass name asString.
		mStr <- aMth name asString.
		aStr <- cStr , '.' , mStr , '.st'.
		"(aFile <- File name: aStr mode: 'w') open."
		aFile putChunk: '{'.
		aFile putChunk: cStr , ' methods'.
		aFile putChunk: aMth trimmedText.
		aFile putChunk: '}'.
		aFile close
	]

	class>>fileOutMethodsOn: aFile [	| sorted |
		" file out all methods "
		(self basicAt: 3) isNil ifTrue: [
			methods <- Dictionary new ].	"fix"
		(self basicAt: 3) isEmpty ifFalse: [
			sorted <- methods sort: [ :x :y |
				x name asString < y name asString ].
			aFile putChunk: '{'.
			aFile putChunk: name asString , ' methods'.
			sorted do: [ :y |
				aFile putChunk: y trimmedText ].
			aFile putChunk: '}' ]
	]

	class>>install: aMethod [	| sel old |	"fix?"
		sel <- aMethod name.
		old <- self methodNamed: sel.	"avoid GC lossage?"
		(self basicAt: 3) at: sel put: aMethod.
		"<38 sel self>."	"primFlushCache"
		self logMethod: aMethod
	]

	class>> (Integer) instanceSize [
		^ (self basicAt: 4)
	]

	class>>logMethod: aMethod [
		'{' logChunk.
		(self name asString , ' methods') logChunk.
		aMethod trimmedText logChunk.
		'}' logChunk
	]

	class>>methodNamed: name [
		(self basicAt: 3) notNil ifTrue: [
		((self basicAt: 3) includesKey: name)
			ifTrue: [ ^ (self basicAt: 3) at: name ] ].
		((self basicAt: 2) notNil)
			ifTrue: [ ^ (self basicAt: 2) methodNamed: name ].
		^ nil
	]

	class>>methods [
		^ (self basicAt: 3)
	]

	class>> (String) name [
		^ (self basicAt: 1)
	]


	class>>name: aString [
		name <- aString
	]

	class>>name: nSym instanceSize: iInt methods: mDict superClass: sClass variables: vArray [
		name <- nSym.
		instanceSize <- iInt.
		methods <- mDict.
		superClass <- sClass.
		variables <- vArray
	]

	class>> (instancetype) new [
		^ self primOrefs: (self basicAt: 4)
	]

	class>> (instancetype) new: size [
		^ self primOrefs: size
	]

	class>>newMethod: aStr [	| m |
		(m <- self doEdit: aStr) notNil ifTrue: [
			self install: m ]
	]

	class>> (id) primBytes: size [
		"create a new block, set its class"
		^ <#classOfPut <#newBytes size> self>
	]

	class>> (id) primOrefs: size [
		"create a new block, set its class"
		^ <#classOfPut <#newOops size> self>
	]

	class>> (String) printString [
		^ (self basicAt: 1) asString
	]

	class>>readMethods [
		[ smalltalk inquire: 'Add a method (yn) ? ' ]
			whileTrue: [ self addMethod ]
	]

	class>>removeMethod: name [	| m |
		m <- self methodNamed: name.
		(m notNil and: [m methodClass == self]) ifTrue: [
			(self basicAt: 3) removeKey: name
			"<38 name self>" ]	"primFlushCache"
		ifFalse: [
			'no such method' print ]
	]

	class>>respondsTo [	| theSet |
		theSet <- Dictionary new.
		self upSuperclassChain:
			[:x | theSet addAll: x methods ].
		^ theSet
	]

	class>>superClass [
		^ (self basicAt: 2)
	]

	class>>superClass: aClass [
		(self basicAt:2 put: aClass)
	]

	class>>upSuperclassChain: aBlock [
		aBlock value: self.
		((self basicAt: 2) notNil)
			ifTrue: [ (self basicAt: 2) upSuperclassChain: aBlock ]
	]

	class>>variables [
		^ (self basicAt: 5)
	]

	class>>variables: nameArray [
		variables <- nameArray.
		instanceSize <- (self basicAt: 2) instanceSize + nameArray size
	]

	class>>viewMethod: methodName [	| m |
		m <- self methodNamed: methodName.
		(m notNil)
			ifTrue: [ m signature print.  m trimmedText print ]
			ifFalse: [ 'no such method' print ]
	]

	class>>watch: name [	| m |
		m <- self methodNamed: name.
		(m notNil)
			ifTrue: [ ^ m watch:
				[:a | ('executing ', name) print. a print] ]
			ifFalse: [ ^ 'no such method' ]
	]

	(String) asString [
		^ self printString
	]

	(T) assign: (Symbol)name value: (T)val <T> [
		^ name assign: val
	]

	(id) basicAt: index [
		^ <#basicAt self index>
	]

	(id) basicAt: index put: value [
		^ <#basicAtPut self index value>
	]

	(Integer) basicSize [
		^ <#size self>
	]

	(self class) class [
		^ <#class self>
	]

	(self) copy [
		^ self shallowCopy
	]

	(self) deepCopy [	| newObj |
		newObj <- self class new.
		(1 to: self basicSize) do:
			[:i | newObj basicAt: i put: (self basicAt: i) copy].
		^ newObj
	]

	(self) display [
		('(Class ', self class, ') ' , self printString ) print
	]

	(Integer) hash [
		^ <#hash self>
	]

	(Boolean) isFloat [
		^ false
	]

	(Boolean) isFraction [
		^ false
	]

	(Boolean) isInteger [
		^ false
	]

	(Boolean) isKindOf: aClass [
		self class upSuperclassChain:
			[:x | (x == aClass) ifTrue: [ ^ true ] ].
		^ false
	]

	(Boolean) isLongInteger [
		^ false
	]

	(Boolean) isMemberOf: aClass [
		^ self class == aClass
	]

	(Boolean) isNil [
		^ false
	]

	(Boolean) isNumber [
		^ false
	]

	(Boolean) isShortInteger [
		^ false
	]

	(self) message: m notRecognizedWithArguments: a [
		^ VM error: (self class printString), ' did not understand selector ',
			(m printString)
	]

	(Boolean) notNil [
		^ true
	]

	(self) print [
		self printString print
	]

	(self) printString [
		^ self class printString
	]

	(Boolean) respondsTo: message [
		self class upSuperclassChain:
			[:c | (c methodNamed: message) notNil
					ifTrue: [ ^ true ]].
		^ false
	]

	(self) shallowCopy [	| newObj |
		newObj <- self class new.
		(1 to: self basicSize) do:
			[:i | newObj basicAt: i put: (self basicAt: i) ].
		^ newObj
	]

	(self) yourself [
		^ self
	]

	(Boolean) = aValue [
		^ self == aValue
	]

	(Boolean) == aValue [
		^ <#oopEq self aValue>
	]

	(Boolean) ~= aValue [
		^ self ~~ aValue
	]

	(Boolean) ~~ aValue [
		^ (self == aValue) not
	]

]

Object subclass: Smalltalk [ ]