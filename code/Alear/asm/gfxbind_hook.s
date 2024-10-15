.global _gfxbind_hook_naked
_gfxbind_hook_naked:
    stdu %r1, -0x100(%r1)
    mflr %r0
    std %r0, 0x110(%r1)

    # Call the render hook

    mr %r3, %r28 # RGfxMaterial*
    mr %r4, %r31 # program
    # rldicl %r5, %r22, 0x0, 0x20
    mr %r5, %r29

    # Try to fetch instance from the instance color variable, sneaky!
    li %r6, 0
    cmpwi %cr7, %r19, 0
    beq %cr7, gfxbind_call_hook
    addi %r6, %r19, -0x80

gfxbind_call_hook:
    std %r2, 0x28(%r1)
    lis %r7, _Z19OnGfxMaterialBindedP12RGfxMaterialP10_CGprogramjP13CMeshInstance@h 
    ori %r7, %r7, _Z19OnGfxMaterialBindedP12RGfxMaterialP10_CGprogramjP13CMeshInstance@l
    lwz %r2, 0x4(%r7)
    bl ._Z19OnGfxMaterialBindedP12RGfxMaterialP10_CGprogramjP13CMeshInstance

    ld %r2, 0x28(%r1)

    ld %r0, 0x110(%r1)
    mtlr %r0
    addi %r1, %r1, 0x100

    # Return to normal execution
    cmpwi %cr7, %r30, 0x0
    ba 0x000997cc
