Collection subclass: Interval [
	| lower upper step |

	class>>lower: lValue upper: uValue step: sValue [
		^ self new lower: lValue upper: uValue step: sValue
	]

	do: aBlock [		| current |
		current <- lower.
		(step > 0) 
			ifTrue: [ [ current <= upper ] whileTrue:
					[ aBlock value: current.
			  		current <- current + step ] ]
			ifFalse: [ [ current >= upper ] whileTrue:
					[ aBlock value: current.
					current <- current + step ] ]
	]

	lower: aValue [
		lower <- aValue
	]

	lower: lValue upper: uValue step: sValue [
		lower <- lValue.
		upper <- uValue.
		step <- sValue
	]

	step: aValue [
		step <- aValue
	]

	upper: aValue [
		upper <- aValue
	]

]