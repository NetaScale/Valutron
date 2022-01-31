Object subclass: Link [
	| nextLink |

	nextLink [
		^ nextLink
	]

	nextLink: aLink [
		nextLink <- aLink
	]
]