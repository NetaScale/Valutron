Behavior subclass: Metaclass [

	class>>metaName: nSym instanceSize: iInt methods: mDict superClass: sClass variables: vArray [
		^ self basicNew
			name: nSym
			instanceSize: iInt
			methods: mDict
			superClass: sClass
			variables: vArray
	]

		    instName: nSym 
		instanceSize: iInt 
		     methods: mDict
		  superClass: sClass
		   variables: vArray [
		^ self basicNew
			name: nSym
			instanceSize: iInt
			methods: mDict
			superClass: sClass
			variables: vArray
	]

	subClasses [
		^ classes inject: List new
			into: [:x :y | (y class superClass == self)
						ifTrue: [ x add: y class ]. x ]
	]
]