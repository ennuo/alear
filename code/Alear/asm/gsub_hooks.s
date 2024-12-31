.global _gsub_rlst_hook
_gsub_rlst_hook:
    std %r2, 0x28(%r1)
    lis %r5, _Z25OnLoadSubstTablesFinishedv@h      
    ori %r5, %r5, _Z25OnLoadSubstTablesFinishedv@l
    lwz %r2, 0x4(%r5)
    bl ._Z25OnLoadSubstTablesFinishedv
    ld %r2, 0x28(%r1)

    ba 0x000b94e8
