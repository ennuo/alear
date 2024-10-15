.global _portal_hook_naked
_portal_hook_naked:
    stdu %r1, -0x100(%r1)
    mflr %r0
    std %r0, 0x110(%r1)

    # Call the render hook
    std %r2, 0x28(%r1)
    lis %r5, _Z19OnWorldRenderUpdatev@h 
    ori %r5, %r5, _Z19OnWorldRenderUpdatev@l
    lwz %r2, 0x4(%r5)
    bl ._Z19OnWorldRenderUpdatev

    ld %r2, 0x28(%r1)

    ld %r0, 0x110(%r1)
    mtlr %r0
    addi %r1, %r1, 0x100

    # Return to normal execution
    stdu %r1, -0x100(%r1)
    ba 0x00206178
