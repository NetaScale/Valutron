Object subclass: VM [

	class>>error: aString [
		<162 aString>
	]

	echo [
		" enable - disable echo input "
		"echoInput <- echoInput not"
		^ 'fixMe'
	]

	error: aString [
		" print a message, and remove current process "
		"stderr print: aString."
		^ 'fixMe'
		scheduler currentProcess yourself; trace; terminate
	]

	getPrompt: aString [
		"stdout printNoReturn: aString.
		^ stdin getString"
		^ 'fixMe'
	]

	inquire: aString [	| response |
		response <- self getPrompt: aString.
		response isNil
			ifTrue: [ ^ false ].
		^ 'Yy' includes: (response at: 1 ifAbsent: [])
	]

	perform: message withArguments: args [
		^ self perform: message withArguments: args
			ifError: [ self error: 'cant perform' ]
	]

	perform: message withArguments: args ifError: aBlock [
			| receiver method |
		receiver <- args at: 1 ifAbsent: [ ^ aBlock value ].
		method <- receiver class methodNamed: message.
		^ method notNil
			ifTrue: [ method executeWith: args ]
			ifFalse: aBlock
	]

	saveImage [
		self saveImage: (self getPrompt: 'type image name: ').
		^ 'done'
	]

	saveImage: name	 [
		"
		scheduler critical: [
			first get rid of our own process 
			scheduler removeProcess: scheduler currentProcess.
	(File name: name open: 'w')
			yourself;
			saveImage;
			close ]
			55.
		]"
		55.
	]

	shutDown [
		"files do: [ :e |
			e notNil ifTrue: [
				(#('stdin' 'stdout' 'stderr') includes: e name) ifFalse: [
					e close ] ] ]"
		^ 'fixme'
	]

	watch [
		^ <5>
	]

	exit [
		^ <9>
	]


]