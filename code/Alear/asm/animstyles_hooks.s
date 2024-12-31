.global _animstyles_hook
_animstyles_hook:
    rldicl %r3, %r25, 0x0, 0x20

    # Load the subtype
    lwz %r6, 0xb4(%r5)
    cmpwi %cr7, %r6, 16384
    beq %cr7, _SetInventoryAnimationStyle
    # Call standard set inventory costume
    ba 0x0038ae7c
_SetInventoryAnimationStyle:
    std %r2, 0x28(%r1)
    lis %r6, _Z26SetInventoryAnimationStyleP16CPoppetInventoryP6CThingP5RPlan@h      
    ori %r6, %r6, _Z26SetInventoryAnimationStyleP16CPoppetInventoryP6CThingP5RPlan@l
    lwz %r2, 0x4(%r6)
    bl ._Z26SetInventoryAnimationStyleP16CPoppetInventoryP6CThingP5RPlan
    ld %r2, 0x28(%r1)

    ba 0x0038adc8
