.global _popit_update_menu_shape_hook
_popit_update_menu_shape_hook:
    mr %r3, %r30
    mr %r4, %r29

    std %r2, 0x28(%r1)
    lis %r7, _Z17CustomUpdateShapeP13CPoppetBubbleb@h      
    ori %r7, %r7, _Z17CustomUpdateShapeP13CPoppetBubbleb@l
    lwz %r2, 0x4(%r7)
    bl ._Z17CustomUpdateShapeP13CPoppetBubbleb
    ld %r2, 0x28(%r1)

    ba 0x0035b230
