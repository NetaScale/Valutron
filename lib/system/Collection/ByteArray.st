Array subclass: ByteArray [

	class>>basicNew: size [
		^ self primBytes: size
	]

	class>>new: size [
		^ self primBytes: size
	]


    asByteArray [
		^ self
	]

	asString [	| newString i |
		newString <- String new: self size.
		i <- 0.
		self do: [:x | i <- i + 1. newString at: i put: x asCharacter].
		^ newString
	]

	basicAt: index [
		^ <26 self index>
	]

	basicAt: index put: value [
		^ ((value isMemberOf: Integer) and: [value between: 0 and: 255])
			ifTrue: [ <32 self index value > ]
			ifFalse: [ value print. VM error: 
				'assign illegal value to ByteArray']
	]

	logChunk [
		^ <154 self>
	]

	size: value [
		^ <22 <59 value> ByteArray>
	]


]