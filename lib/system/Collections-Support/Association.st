Magnitude subclass: Association [
    | key value |

    class>>key: newKey value: newValue [
        "Answer a new instance of me with the arguments as the key and
        value of the association."

        ^ self new key: newKey value: newValue
    ]

    key: aKey value: anObject [
        "Store the arguments as the variables of the receiver."

        key <- aKey.
        value <- anObject
    ]

    key [
        "Answer the key of the receiver."
        ^ key
    ]

    key: anObject [
        "Store the argument, anObject, as the key of the receiver."
        key <- anObject
    ]

    value [
        "Answer the value of the receiver."
        ^ value
    ]

    value: anObject [
        "Store the argument, anObject, as the value of the receiver."
        value <- anObject
    ]

    = anAssociation [
        ^ anAssociation key = key and: [anAssociation value = value]
    ]
]