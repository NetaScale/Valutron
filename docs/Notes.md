
Types
-----

- Block copying should happen whenever an isSubTypeOf() is invoked on a generic
  block. isSubTypeOf() ought probably to allow for returning a new type.
    - Troubles: can we have a generic block type nested in another type? I think
    so.