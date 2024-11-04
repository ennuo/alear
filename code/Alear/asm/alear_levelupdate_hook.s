.global _alear_levelupdate_hook
_alear_levelupdate_hook:
    std %r2, 0x28(%r1)
    lis %r5, _Z13OnUpdateLevelv@h      
    ori %r5, %r5, _Z13OnUpdateLevelv@l
    lwz %r2, 0x4(%r5)
    bl ._Z13OnUpdateLevelv
    ld %r2, 0x28(%r1)

    li %r3, 1
    ba 0x00015878
