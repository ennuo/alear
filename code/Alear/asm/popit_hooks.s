.global _popit_update_menu_shape_hook
_popit_update_menu_shape_hook:
    mr %r3, %r30
    mr %r4, %r29

    std %r2, 0x28(%r1)
    lis %r7, _Z17CustomUpdateShapeP13CPoppetBubbleb@h      
    ori %r7, %r7, _Z17CustomUpdateShapeP13CPoppetBubbleb@l
    lwz %r2, 0x4(%r7)
    bl ._Z17CustomUpdateShapeP13CPoppetBubbleb
    ld %r2, 0x28(%r1)

    ba 0x0035b230

.global _popit_isitemselected_hook
_popit_isitemselected_hook:
    rldicl %r3, %r0, 0x0, 0x20
    mr %r4, %r25

    std %r2, 0x28(%r1)
    lis %r7, _Z14IsItemSelectedP13RLocalProfileP14CInventoryItem@h      
    ori %r7, %r7, _Z14IsItemSelectedP13RLocalProfileP14CInventoryItem@l
    lwz %r2, 0x4(%r7)
    bl ._Z14IsItemSelectedP13RLocalProfileP14CInventoryItem
    ld %r2, 0x28(%r1)

    ba 0x003800c8

.global _popit_dopoppetsection_hook
_popit_dopoppetsection_hook:
    mflr %r0
    stw %r0, 0x28(%r1)
    stw %r2, 0x2c(%r1)

    lis %r2, _Z21CustomDoPoppetSectionP12CPoppetChildP11PageSectionR15SectionSettingsbyP20CInventoryCollectionj15EPoppetItemArgsRjRbS9_PKt@h      
    ori %r2, %r2, _Z21CustomDoPoppetSectionP12CPoppetChildP11PageSectionR15SectionSettingsbyP20CInventoryCollectionj15EPoppetItemArgsRjRbS9_PKt@l
    lwz %r2, 0x4(%r2)
    bl ._Z21CustomDoPoppetSectionP12CPoppetChildP11PageSectionR15SectionSettingsbyP20CInventoryCollectionj15EPoppetItemArgsRjRbS9_PKt
    
    lwz %r2, 0x2c(%r1)
    lwz %r0, 0x28(%r1)

    mtlr %r0
    blr
