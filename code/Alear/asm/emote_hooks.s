.global _emote_hook
_emote_hook:
    mr %r3, %r31

    std %r2, 0x28(%r1)
    lis %r5, _Z17OnTriggerIdleAnimR12CSackBoyAnim@h      
    ori %r5, %r5, _Z17OnTriggerIdleAnimR12CSackBoyAnim@l
    lwz %r2, 0x4(%r5)
    bl ._Z17OnTriggerIdleAnimR12CSackBoyAnim
    ld %r2, 0x28(%r1)

    ba 0x000f6148

.global _sbanim_emote_init_hook
_sbanim_emote_init_hook:
    mr %r3, %r31

    std %r2, 0x28(%r1)
    lis %r5, _Z24OnInitializeSackboyAnimsR12CSackBoyAnim@h      
    ori %r5, %r5, _Z24OnInitializeSackboyAnimsR12CSackBoyAnim@l
    lwz %r2, 0x4(%r5)
    bl ._Z24OnInitializeSackboyAnimsR12CSackBoyAnim
    ld %r2, 0x28(%r1)

    mr %r3, %r29
    ba 0x000fa950

.global _emote_select_hook
_emote_select_hook:
    lwz %r3, 0x34(%r31)

    std %r2, 0x28(%r1)
    lis %r5, _Z11IsEmoteItem5CGUID@h      
    ori %r5, %r5, _Z11IsEmoteItem5CGUID@l
    lwz %r2, 0x4(%r5)
    bl ._Z11IsEmoteItem5CGUID
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 0x0 
    beq %cr7, _NotAnEmote
    ba 0x0038a47c
_NotAnEmote:
    ba 0x0038a574

.global _sbanim_update_emote_sounds_hook
_sbanim_update_emote_sounds_hook:
    # Back up the arguments on the stack
    std %r3, 0x40(%r1)
    std %r4, 0x48(%r1)
    std %r5, 0x50(%r1)

    # Call custom animation sound handler
    std %r2, 0x28(%r1)
    lis %r6, _Z23OnUpdateAnimationSoundsR12CSackBoyAnimii@h      
    ori %r6, %r6, _Z23OnUpdateAnimationSoundsR12CSackBoyAnimii@l
    lwz %r2, 0x4(%r6)
    bl ._Z23OnUpdateAnimationSoundsR12CSackBoyAnimii
    ld %r2, 0x28(%r1)

    # Restore argument registers
    ld %r3, 0x40(%r1)
    ld %r4, 0x48(%r1)
    ld %r5, 0x50(%r1)

    # Branch back to normal animation sounds
    lwz %r6, 0x4(%r3)
    ba 0x000ec728
