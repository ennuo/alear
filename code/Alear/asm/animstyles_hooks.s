.include "asm/macros/fnptr.s"

.global _animstyles_hook
_animstyles_hook:
    rldicl %r3, %r25, 0x0, 0x20

    # Load the subtype
    lwz %r6, 0xb4(%r5)
    cmpwi %cr7, %r6, 16384
    beq %cr7, _SetInventoryAnimationStyle
    rlwinm %r6, %r6, 0x0, 0xf, 0xf
    cmpwi %cr7, %r6, 0
    bne %cr7, _SetInventoryMorph
    # Call standard set inventory costume
    ba 0x0038ae7c
_SetInventoryMorph:
    call _Z17SetInventoryMorphP16CPoppetInventoryP6CThingPK5RPlan
    ba 0x0038adc8
_SetInventoryAnimationStyle:
    call _Z26SetInventoryAnimationStyleP16CPoppetInventoryP6CThingP5RPlan
    ba 0x0038adc8
