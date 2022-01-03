Object subclass: Symbol [

    apply: args [
		^ self apply: args ifError: [ 'does not apply' ]
	]

    apply: args ifError: aBlock [
		^ smalltalk perform: self withArguments: args ifError: aBlock
	]

	asString [
		" catenation makes string and copy automatically "
		^ <24 self ''>
	]

	asSymbol [
		^ self
	]

	assign: value [
		<27 self value>. ^ value
	]

	copy [
		^ self
	]

	printString [
		^ '#' , self asString
	]

	respondsTo [
		^ classes inject: Set new
			into: [:x :y | ((y methodNamed: self) notNil)
						ifTrue: [ x add: y]. x]
	]

	stringHash [
		^ <82 self>
	]

	value [
		^ <87 self>
	]


]