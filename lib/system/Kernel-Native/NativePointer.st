ByteArray subclass: NativePointer [
    "Represents a pointer to a native address. ITs purpose is to make it
     absolutely explicit that some given variable stores a native pointer."

    class>>new [
        ^ self new: 8 "FIXME: use platform word size"
    ]

]