
Types
-----

- Block copying should happen whenever an isSubTypeOf() is invoked on a generic
  block. isSubTypeOf() ought probably to allow for returning a new type.
    - Troubles: can we have a generic block type nested in another type? I think
    so.

GC
--

 - Any globals MUST be fixed with MPS - otherwise when things are copied, they
    break.
 - 21/01/2022: Huge problem (within opLdaBlockCopy) where a variable was,
   seemingly, not visible to MPS's stack/regs scanner (stored as offset or
   indirect as a result of optimisations?) - solved by explicitly storing that
   variable into a heap-allocated structure.
 -  Thought that was solved - I was wrong. I think that some of the problem
    might come from the scan function accessing into objects other than the
    object it is scanning, i.e. isa() (the new code checks stuff like whether
    something is a stack-allocated object in the FIXOOP macro, and segfaults on
    FIXOOP(obj->isa()); sometimse )
    - proc->context is not fixed, neither is context prevContext (nor Block
      homeMethodContext), big problems if the stack is moved by MPS - that is
      the current issue.