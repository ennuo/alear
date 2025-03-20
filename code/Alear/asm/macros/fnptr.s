.macro call mangled_name
    std %r2, 0x28(%r1)
    lis %r2, \mangled_name@h
    ori %r2, %r2, \mangled_name@l
    lwz %r2, 0x4(%r2)
    bl .\mangled_name
    ld %r2, 0x28(%r1)
.endm

.macro set_return_pointer pointer
    .set FunctionReturnPointer, \pointer
.endm

.macro ret
    ba FunctionReturnPointer
.endm


.macro create_hook name, return_address
.global _\name
_\name:
    set_return_pointer \return_address
.endm
