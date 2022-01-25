"A StackFrame acts as a proxy to an inline Context within a Process' stack."
Object subclass: StackFrame [
	| process bpOffset |

	shortName [
		| className |
		className <- self receiver class name asString.
		^ self isBlockContext ifTrue: [
			'[] in ', className, '>>', homeMethodContext methodOrBlock message
		] ifFalse: [
			className, '>>', methodOrBlock message
		]
	]

	"accessors - 2 is added to account for the header"

	methodOrBlock [
		^ process basicAt: bpOffset + 4.
	]

	homeMethodContext [
		^ process basicAt: bpOffset + 5.
	]

	receiver [
		^ process basicAt: bpOffset + 10.
	]
]