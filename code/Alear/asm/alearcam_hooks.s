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

.global _alearcam_renderworld_hook
_alearcam_renderworld_hook:
    std %r2, 0x28(%r1)
    lis %r5, _Z27OnRunPipelinePostProcessingv@h      
    ori %r5, %r5, _Z27OnRunPipelinePostProcessingv@l
    lwz %r2, 0x4(%r5)
    bl ._Z27OnRunPipelinePostProcessingv
    ld %r2, 0x28(%r1)

    lwz %r31, 0x9b4(%r2)
    ba 0x001e6b90
