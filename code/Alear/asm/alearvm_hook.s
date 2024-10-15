.global _alearvm_hook_naked
_alearvm_hook_naked:
    # Move the execution state into the first argument register
    mr %r3, %r18

    # Call the extended handler function
    std %r2, 0x28(%r1)
    lis %r5, _Z13AlearHandleVMP14ExecutionState@h      
    ori %r5, %r5, _Z13AlearHandleVMP14ExecutionState@l
    lwz %r2, 0x4(%r5)
    bl ._Z13AlearHandleVMP14ExecutionState
    ld %r2, 0x28(%r1)

    # Break out of the switch case
    ba 0x00189c14
