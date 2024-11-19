.global _outfitlist_onitemadded
_outfitlist_onitemadded:
    rlwinm %r24, %r0, 0x1, 0x1f, 0x1f
    cmpwi %cr7, %r24, 0
    beq %cr7, SkipOutfitCheck
    
    addi %r3, %r31, -0x8c
    mr %r4, %r29

    std %r2, 0x28(%r1)
    lis %r6, _Z15OnItemCollectedR7CPlayerRK2CPI5RPlanE@h      
    ori %r6, %r6, _Z15OnItemCollectedR7CPlayerRK2CPI5RPlanE@l
    lwz %r2, 0x4(%r6)
    bl ._Z15OnItemCollectedR7CPlayerRK2CPI5RPlanE
    ld %r2, 0x28(%r1)


SkipOutfitCheck:
    ba 0x0040c81c
