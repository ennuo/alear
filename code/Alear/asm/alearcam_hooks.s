.global _alearcam_update_hook
_alearcam_update_hook:
    std %r2, 0x28(%r1)
    lis %r5, _Z26OnPredictionOrRenderUpdatev@h      
    ori %r5, %r5, _Z26OnPredictionOrRenderUpdatev@l
    lwz %r2, 0x4(%r5)
    bl ._Z26OnPredictionOrRenderUpdatev
    ld %r2, 0x28(%r1)

    lis %r9, 0x10
    ba 0x00014ad0