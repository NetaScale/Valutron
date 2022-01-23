Object subclass: File [
	| (NativePointer)fileStar (Symbol)mode |

	(self) initWithDescriptor: (Integer)fd mode: (Symbol)aMode [
		fileStar <- <#fileDescToFileStar fd aMode>.
		mode <- aMode.
		^ self
	]

	cr [
		self nextPut: (Character lf).
	]

	<< anObject [
		^ self nextPut: anObject
	]

	nextPut: anObject [
		<#fileStarPut fileStar anObject>.
		^ self
	]
]