Behavior subclass: Class [

	fileOut [		| f |
		" file out whole class on class.st "
		(f <- File name: (name asString,'.st') mode: 'w') open.
		self fileOutOn: f.
		f close
	]

	fileOutClassOn: aFile [	| dlm pad buf buf2 |
		dlm <- 10 asCharacter asString.
		pad <- 9 asCharacter asString.
		buf <-	superClass isNil ifTrue: [
				'nil' ]
			ifFalse: [
				superClass name asString ].
		buf <- buf , dlm , pad.
		buf <- buf , 'subclass: ' , self name printString.
		buf <- buf , dlm , pad.
		buf2 <- ''.
		variables notNil ifTrue: [
			variables inject: '' into: [ :p :v |
				buf2 <- buf2 , p , v.
				' ' ] ].
		buf <- buf , 'instanceVariableNames: ' , buf2 printString.
		aFile putChunk: buf
	]

	fileOutOn: aFile [
		" file out class description "
		self fileOutClassOn: aFile.
		self class fileOutMethodsOn: aFile.
		self fileOutMethodsOn: aFile
	]

	subClasses [
		^ classes inject: List new
			into: [:x :y | (y superClass == self)
						ifTrue: [ x add: y]. x ]
	]

	subclass: aSymbol instanceVariableNames: aString [
		| newMeta varArray newClass |
		newMeta <- Metaclass
			metaName: (aSymbol asString , 'Meta') asSymbol
			instanceSize: self class instanceSize
			methods: Dictionary new
			superClass: self class
			variables: (Array primOrefs: 0).
		varArray <- aString words: [ :x | x isAlphaNumeric ].
		newClass <- newMeta
			instName: aSymbol
			instanceSize: self instanceSize + varArray basicSize
			methods: Dictionary new
			superClass: self
			variables: varArray.
		newMeta name assign: newMeta.
		aSymbol assign: newClass.
		classes at: aSymbol put: newClass.
		^ newClass
	]
]