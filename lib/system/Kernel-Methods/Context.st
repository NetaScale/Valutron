"only here for the convenience of the VM"
Object subclass: Context [
	| (Integer)prevBP
	  (Method | Block)methodOrBlock
	  (Integer) homeMethodContext
	  (ByteArray)bytecode
	  (Array)heapVars
	  (Array)parentHeapVars
	  (Integer)programCounter
	  (id)reg0 |
]