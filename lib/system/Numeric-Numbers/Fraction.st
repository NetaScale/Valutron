Number subclass: Fraction [
	| (Number)top (Number)bottom |

	class>>top: tNum bottom: bNum [
		^ self new top: tNum bottom: bNum
	]


	(id) * (Number)f [
		f isFraction
			ifTrue: [ ^ (top * f top) / (bottom * f bottom) ]
			ifFalse: [ ^ super * f ]
	]

	(id) + (Number)f [
		f isFraction
			ifTrue: [ ^ ((top * f bottom) + (bottom * f top)) /
					(bottom * f bottom) ]
			ifFalse:[ ^ super + f ]
	]

	(id) - (Number)f [
		f isFraction
			ifTrue: [ ^ ((top * f bottom) - (bottom * f top)) /
					(bottom * f bottom) ]
			ifFalse:[ ^ super - f ]
	]

	(id) / (Number)f [
		^ self * f reciprocal
	]

	(Boolean) < (Number)f [
		f isFraction
			ifTrue: [ ^ (top * f bottom) < (bottom * f top) ]
			ifFalse:[ ^ super < f ]
	]

	(Boolean) = (Number)f [
		f isFraction
			ifTrue: [ ^ (top = f top) and: [ bottom = f bottom ] ]
			ifFalse: [ ^ super = f ]
	]

	(id) abs [
		^ top abs / bottom
	]

	(Float) asFloat [
		" convert to a floating point number "

		^ top asFloat / bottom asFloat
	]

	(id) bottom  [
		^ bottom
	]

	(Fraction) coerce: (Number)x [
		" coerce a value into being a fraction "

		^ x asFraction
	]

	(Integer) generality [
		" generality value - used in mixed type arithmetic "
		^ 5
	]

	(Boolean) isFraction [
		^ true
	]

	(id) ln [
		^ (top ln) - (bottom ln)
	]

	(String) printString [
		^ top printString, '/', bottom printString
	]

	(id) raisedTo: (Number)x [
		^ (top raisedTo: x) / (bottom raisedTo: x)
	]

	(id) reciprocal [
		^ bottom / top
	]

	(id) top [
		^ top
	]

	(self) top: (Number)tNum bottom: (Number)bNum [
		top <- tNum.
		bottom <- bNum
	]

	(id) truncated [
		" convert to an integer rounded towards zero "
		^ top quo: bottom
	]

	(self) with: (Number)t over: (Number)b [
		" initialization "

		top <- t.
		bottom <- b
	]

]