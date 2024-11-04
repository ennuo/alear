.global _beslapped_hook
_beslapped_hook:
    lwz %r3, 0x8(%r3)
    mr %r4, %r26 # by
    # force is already in %v2
    addi %r7, %r1, 0x80 # pos3d
    mr %r8, %r31 # side

    # Call the extended handler function
    std %r2, 0x28(%r1)
    lis %r5, _Z15OnPlayerSlappedP6CThingS0_2v2PKS1_b@h      
    ori %r5, %r5, _Z15OnPlayerSlappedP6CThingS0_2v2PKS1_b@l
    lwz %r2, 0x4(%r5)
    bl ._Z15OnPlayerSlappedP6CThingS0_2v2PKS1_b
    ld %r2, 0x28(%r1)

    ba 0x000e6e5c
