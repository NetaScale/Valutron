nil subclass: Object [
	| |
	| name superClass methods nstSize nstVars |

	"Behaviour"
	class>>addMethod [	| m |
		(m <- self doEdit: '') notNil ifTrue: [
			self install: m ]
	]

	class>>basicNew [
		^ self primOrefs: (self basicAt: 4)
	]

	class>>basicNew: size [
		^ self primOrefs: size
	]

	class>>display [
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
		  ans <- Parser new parse: tmp in: self.
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
		(aFile <- File name: aStr mode: 'w') open.
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
		<38 sel self>.	"primFlushCache"
		self logMethod: aMethod
	]

	class>>instanceSize [
		^ (self basicAt: 4)
	]

	class>>logMethod: aMethod [
		'{' logChunk.
		(self name asString , ' methods') logChunk.
		aMethod trimmedText logChunk.
		'}' logChunk
	]

	class>>methodNamed: name [
		((self basicAt: 3) includesKey: name)
			ifTrue: [ ^ (self basicAt: 3) at: name ].
		((self basicAt: 2) notNil)
			ifTrue: [ ^ (self basicAt: 2) methodNamed: name ].
		^ nil
	]

	class>>methods [
		^ (self basicAt: 3)
	]

	class>>name [
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

	class>>new [
		^ self primOrefs: (self basicAt: 4)
	]

	class>>new: size [
		^ self primOrefs: size
	]

	class>>newMethod: aStr [	| m |
		(m <- self doEdit: aStr) notNil ifTrue: [
			self install: m ]
	]

	class>>primBytes: size [
		"create a new block, set its class"
		^ <22 <59 size> self>
	]

	class>>primOrefs: size [
		"create a new block, set its class"
		^ <22 <58 size> self>
	]

	class>>printString [
		^ (self basicAt: 1) asString
	]

	class>>readMethods [
		[ smalltalk inquire: 'Add a method (yn) ? ' ]
			whileTrue: [ self addMethod ]
	]

	class>>removeMethod: name [	| m |
		m <- self methodNamed: name.
		(m notNil and: [m methodClass == self]) ifTrue: [
			(self basicAt: 3) removeKey: name.
			<38 name self> ]	"primFlushCache"
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

	= aValue [
		^ self == aValue
	]

	== aValue [
		^ <21 self aValue>
	]

	asString [
		^ self printString
	]

	assign: name value: val [
		^ name assign: val
	]

	basicAt: index [
		^ <25 self index>
	]

	basicAt: index put: value [
		^ <31 self index value>
	]

	basicSize [
		^ <12 self>
	]

	class [
		^ <11 self>
	]

	copy [
		^ self shallowCopy
	]

	deepCopy [	| newObj |
		newObj <- self class new.
		(1 to: self basicSize) do: 
			[:i | newObj basicAt: i put: (self basicAt: i) copy].
		^ newObj
	]

	display [
		('(Class ', self class, ') ' , self printString ) print
	]

	hash [
		^ <13 self>
	]

	isFloat [
		^ false
	]

	isFraction [
		^ false
	]

	isInteger [
		^ false
	]

	isKindOf: aClass [
		self class upSuperclassChain:
			[:x | (x == aClass) ifTrue: [ ^ true ] ].
		^ false
	]

	isLongInteger [
		^ false
	]

	isMemberOf: aClass [
		^ self class == aClass
	]

	isNil [
		^ false
	]

	isNumber [
		^ false
	]

	isShortInteger [
		^ false
	]

	message: m notRecognizedWithArguments: a [
		^ VM error: (self class printString), ' did not understand selector ',
			(m printString)
	]

	notNil [
		^ true
	]

	print [
		self printString print 
	]

	printString [
		^ self class printString
	]

	respondsTo: message [
		self class upSuperclassChain: 
			[:c | (c methodNamed: message) notNil
					ifTrue: [ ^ true ]].
		^ false
	]

	shallowCopy [	| newObj |
		newObj <- self class new.
		(1 to: self basicSize) do: 
			[:i | newObj basicAt: i put: (self basicAt: i) ].
		^ newObj
	]

	yourself [
		^ self
	]

	~= aValue [
		^ self ~~ aValue
	]

	~~ aValue [
		^ (self == aValue) not
	]


]