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
			'[] in ', className, '>>', self homeMethodFrame methodOrBlock message
		] ifFalse: [
			className, '>>', self methodOrBlock message
		]
	]

	"accessors - 2 is added to account for the header"

	prevBP [
		^ process stack basicAt: bpOffset + 2
	]

	methodOrBlock [
		^ process stack basicAt: bpOffset + 3.
	]

	homeMethodContext [
		^ process stack basicAt: bpOffset + 4.
	]

	homeMethodFrame [
		^ StackFrame new initWithProcess: process bp: self prevBP
	]

	receiver [
		^ process stack basicAt: bpOffset + 10.
	]

	"enquiries"

	isBlockContext [
		^ self methodOrBlock class == Block
	]
]