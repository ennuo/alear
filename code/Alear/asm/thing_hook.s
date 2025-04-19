.include "asm/macros/fnptr.s"

.global _initextradata_cthing_hook
_initextradata_cthing_hook:
    sth %r0, 0xbe(%r28)

    mr %r3, %r28
    call _ZN6CThing19InitializeExtraDataEv

    ba 0x00020190

.global _destroyextradata_cthing_hook
_destroyextradata_cthing_hook:
    mr %r3, %r28
    call _ZN6CThing16DestroyExtraDataEv    
    lwz %r3, 0x8c(%r28)
    ba 0x000225d4

.global _initextradata_part_switch
_initextradata_part_switch:
    mr %r3, %r27
    call _ZN7PSwitch19InitializeExtraDataEv
    ld %r0, 0x100(%r1)
    ba 0x0005e6ac

.global _initextradata_part_generatedmesh
_initextradata_part_generatedmesh:
    mr %r3, %r28
    call _ZN14PGeneratedMesh19InitializeExtraDataEv
    ld %r0, 0xb0(%r1)
    ba 0x00031f10
