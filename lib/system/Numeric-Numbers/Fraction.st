Number subclass: Fraction [
	| top bottom |

	class>>top: tNum bottom: bNum [
		^ self new top: tNum bottom: bNum
	]


	* f [
		f isFraction
			ifTrue: [ ^ (top * f top) / (bottom * f bottom) ]
			ifFalse: [ ^ super * f ]
	]

	+ f [
		f isFraction
			ifTrue: [ ^ ((top * f bottom) + (bottom * f top)) /
					(bottom * f bottom) ]
			ifFalse:[ ^ super + f ]
	]

	- f [
		f isFraction
			ifTrue: [ ^ ((top * f bottom) - (bottom * f top)) /
					(bottom * f bottom) ]
			ifFalse:[ ^ super - f ]
	]

	/ f [
		^ self * f reciprocal
	]

	< f [
		f isFraction
			ifTrue: [ ^ (top * f bottom) < (bottom * f top) ]
			ifFalse:[ ^ super < f ]
	]

	= f [
		f isFraction
			ifTrue: [ ^ (top = f top) and: [ bottom = f bottom ] ]
			ifFalse: [ ^ super = f ]
	]

	abs [
		^ top abs / bottom
	]

	asFloat [
		" convert to a floating point number "

		^ top asFloat / bottom asFloat
	]

	bottom  [
		^ bottom
	]

	coerce: x [
		" coerce a value into being a fraction "

		^ x asFraction
	]

	generality [
		" generality value - used in mixed type arithmetic "
		^ 5
	]

	isFraction [
		^ true
	]

	ln [
		^ (top ln) - (bottom ln)
	]

	printString [
		^ top printString, '/', bottom printString
	]

	raisedTo: x [
		^ (top raisedTo: x) / (bottom raisedTo: x)
	]

	reciprocal [
		^ bottom / top
	]

	top [
		^ top
	]

	top: tNum bottom: bNum [
		top <- tNum.
		bottom <- bNum
	]

	truncated [
		" convert to an integer rounded towards zero "
		^ top quo: bottom
	]

	with: t over: b [
		" initialization "

		top <- t.
		bottom <- b
	]

]