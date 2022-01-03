Object subclass: UndefinedObject [

	initBot [	| aBlock saveFile saveAns |
		" initialize the initial object image "
		aBlock <- [ files do: [:f | f notNil ifTrue: [ f open ]].
				   echoInput <- false.
				   scheduler run.
				   scheduler <- Scheduler new.
				   systemProcess <- aBlock newProcess ].
		scheduler <- Scheduler new.
		systemProcess <- aBlock newProcess.
		saveFile <- File
			name: 'systemImage'
			open: 'w'.
		saveAns <- saveFile saveImage.
		saveFile close.
		stdout
			yourself;
			printNoReturn: 'saveImage: ';
			print: saveAns printString
	]

	initMid [
		" initialize the initial object image "
		| metaclasses key |
		metaclasses <- Dictionary new.
		symbols binaryDo: [ :x :y |
			(y class == Metaclass) ifTrue: [
				key <- (x asString copyFrom: 1 to: x basicSize - 5) asSymbol.
				metaclasses at: key put: y ] ].
		classes <- Dictionary new.
		symbols binaryDo: [ :x :y |
			((metaclasses at: x ifAbsent: [nil]) == y class) ifTrue: [
				classes at: x put: y ] ]
	]

	initTop [
		" initialize the initial object image "
		files <- Array new: 15.
		(stdin <- File name: 'stdin' mode: 'r') open.
		(stdout <- File name: 'stdout' mode: 'w') open.
		(stderr <- File name: 'stderr' mode: 'w') open.
		editor <- 'vi'
	]

	initialize [
		" initialize the initial object image "
		smalltalk <- Smalltalk new.
		self initTop.
		self initMid.
		self initBot
	]

	isNil [
		^ true
	]

	notNil [
		^ false
	]

	printString [
		^ 'nil'
	]

]