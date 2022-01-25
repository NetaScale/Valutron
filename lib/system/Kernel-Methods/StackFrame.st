"A StackFrame acts as a proxy to the inline context within a Process' stack."
Object subclass: StackFrame [
	| process bpOffset |

	initWithProcess: aProcess bp: bp [
		process <- aProcess.
		bpOffset <- bp.
	]

	shortName [
		| className |
		className <- self receiver class name asString.
		^ self isBlockContext ifTrue: [
			'[] in ', self homeMethodFrame receiver class name, '>>',
			    self homeMethodFrame methodOrBlock message
		] ifFalse: [
			className, '>>', self methodOrBlock message
		]
	]

	"accessors - 2 is added to account for the header"

	prevBP [
		^ bpOffset isNil ifFalse: [
			process stack basicAt: bpOffset + 2
		]
		ifTrue: [ nil ]
	]

	methodOrBlock [
		^ bpOffset isNil ifFalse: [
			process stack basicAt: bpOffset + 3
		]
		ifTrue: [ nil ]
	]

	homeMethodContext [
		^ bpOffset isNil ifFalse: [
			process stack basicAt: bpOffset + 4
		]
		ifTrue: [ nil ]
		]

	homeMethodFrame [
		^ StackFrame new initWithProcess: process bp: self homeMethodContext
	]

	receiver [
		^ bpOffset isNil ifFalse: [
			process stack basicAt: bpOffset + 9
		]
		ifTrue: [ nil ]
	]

	"enquiries"

	isBlockContext [
		^ self methodOrBlock class == Block
	]
]