.global _gooey_frame_clip_hook
_gooey_frame_clip_hook:
    lbz %r8, 166(%r29)
    or %r5, %r5, %r8
    lbz %r8, 167(%r29)
    or %r6, %r6, %r8

    bla 0x002f08f0
    ba 0x002f1d1c

.global _gooey_image_ctor_animated_hook
_gooey_image_ctor_animated_hook:
    mr %r3, %r25

    std %r2, 0x28(%r1)
    lis %r5, _ZN11CGooeyImage19InitializeExtraDataEv@h      
    ori %r5, %r5, _ZN11CGooeyImage19InitializeExtraDataEv@l
    lwz %r2, 0x4(%r5)
    bl ._ZN11CGooeyImage19InitializeExtraDataEv
    ld %r2, 0x28(%r1)

    lwz %r11, 0x6fd4(%r2)
    ba 0x003227dc

.global _sdf_button_animated_hook
_sdf_button_animated_hook:
    std %r2, 0x28(%r1)

    lis %r2, _ZN11CGooeyImage7InitSDFER2CPI9CResourceEj2v2N10Vectormath3Aos7Vector4ES7_@h      
    ori %r2, %r2, _ZN11CGooeyImage7InitSDFER2CPI9CResourceEj2v2N10Vectormath3Aos7Vector4ES7_@l
    lwz %r2, 0x4(%r2)

    bl ._ZN11CGooeyImage7InitSDFER2CPI9CResourceEj2v2N10Vectormath3Aos7Vector4ES7_
    
    ld %r2, 0x28(%r1)
    
    ba 0x00300270
    
.global _gooey_image_update_hook
_gooey_image_update_hook:
    std %r2, 0x28(%r1)
    lis %r5, _ZN11CGooeyImage21UpdateAnimatedTextureEv@h      
    ori %r5, %r5, _ZN11CGooeyImage21UpdateAnimatedTextureEv@l
    lwz %r2, 0x4(%r5)
    bl ._ZN11CGooeyImage21UpdateAnimatedTextureEv
    ld %r2, 0x28(%r1)

    mr %r3, %r22
    mr %r4, %r24
    ld %r0, 0x2f0(%r4)
    ba 0x00322f98
