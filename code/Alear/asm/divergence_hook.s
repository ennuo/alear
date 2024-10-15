.global _divergence_hook
_divergence_hook:
    std %r2, 0x28(%r1)
    lis %r5, _Z25GetDivergenceCheckEnabledv@h      
    ori %r5, %r5, _Z25GetDivergenceCheckEnabledv@l
    lwz %r2, 0x4(%r5)
    bl ._Z25GetDivergenceCheckEnabledv
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 0x1
    beq %cr7, ReturnToDivergenceCheck
    ba 0x000a2408

ReturnToDivergenceCheck:
    # Restore the state before we did any nonsense
    mr %r4, %r28
    mr %r10, %r31
    lwz %r8, 0x4(%r26)
    mulli %r0, %r8, 0x34
    ba 0x000a231c
