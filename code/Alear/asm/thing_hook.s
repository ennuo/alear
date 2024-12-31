.global _initextradata_cthing_hook
_initextradata_cthing_hook:
    sth %r0, 0xbe(%r28)

    mr %r3, %r28

    std %r2, 0x28(%r1)
    lis %r7, _ZN6CThing19InitializeExtraDataEv@h      
    ori %r7, %r7, _ZN6CThing19InitializeExtraDataEv@l
    lwz %r2, 0x4(%r7)
    bl ._ZN6CThing19InitializeExtraDataEv
    ld %r2, 0x28(%r1)

    ba 0x00020190

.global _destroyextradata_cthing_hook
_destroyextradata_cthing_hook:
    rldicl %r29, %r3, 0x0, 0x20

    std %r2, 0x28(%r1)
    lis %r7, _ZN6CThing16DestroyExtraDataEv@h      
    ori %r7, %r7, _ZN6CThing16DestroyExtraDataEv@l
    lwz %r2, 0x4(%r7)
    bl ._ZN6CThing16DestroyExtraDataEv
    ld %r2, 0x28(%r1)
    
    mr %r3, %r29
    ba 0x00022534
