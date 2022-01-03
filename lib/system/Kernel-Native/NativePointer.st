Object subclass: NativePointer [
    "I represent a pointer to a native address. My purpose is to make it
     absolutely explicit that some given variable stores a native pointer,
     because attempting to send a message to a native pointer would cause
     a crash!"
    | ptr |

    class>>ptr: ptr [
        ^ self new ptr: ptr
    ]

    ptr: anOop [
        ptr <- anOop.
    ]
]