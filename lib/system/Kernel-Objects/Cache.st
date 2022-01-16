Object subclass: Cache [
	| (Symbol)selector
	 "variably sized - remainder laid out as:
	 (Object)class (Integer)version (Method)method
	 a nil method means that the result is ineligible for caching" |
]