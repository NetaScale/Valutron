Object subclass: NativeCode [
    | "<ByteArray>" code |
    "Represents compiled C code, stored as a function in `code`.
    The first sizeof(void (*)()) bytes of code are the function pointer.
    The remainder is the rest of that code."

    value [
        ^ <164 self>
    ]
]