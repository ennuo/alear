.global _global_onswapbuffers_hook
_global_onswapbuffers_hook:
    # Call the extended handler function
    std %r2, 0x28(%r1)
    lis %r5, _Z13OnSwapBuffersv@h      
    ori %r5, %r5, _Z13OnSwapBuffersv@l
    lwz %r2, 0x4(%r5)
    bl ._Z13OnSwapBuffersv
    ld %r2, 0x28(%r1)

    # Break out of the switch case
    lwz %r0, 0x0(%r30)
    ba 0x003e41bc

.global _global_webternate_hook
_global_webternate_hook:
    mr %r3, %r28
    
    # Call the extended handler function
    std %r2, 0x28(%r1)
    lis %r5, _Z17OnWebternateSetupP11CWebternate@h      
    ori %r5, %r5, _Z17OnWebternateSetupP11CWebternate@l
    lwz %r2, 0x4(%r5)
    bl ._Z17OnWebternateSetupP11CWebternate
    ld %r2, 0x28(%r1)

    # Break out of the switch case
    ld %r0, 0xb0(%r1)
    ba 0x00234fd0

.global _global_artist_hook
_global_artist_hook:
    mr %r3, %r27

    std %r2, 0x28(%r1)
    lis %r5, _Z12DoArtistSortP14CInventoryView@h      
    ori %r5, %r5, _Z12DoArtistSortP14CInventoryView@l
    lwz %r2, 0x4(%r5)
    bl ._Z12DoArtistSortP14CInventoryView
    ld %r2, 0x28(%r1)

    ba 0x000c2cf4

.global _global_pref_hook
_global_pref_hook:
    mr %r3, %r27

    std %r2, 0x28(%r1)
    lis %r5, _Z16DoPreferenceSortP14CInventoryView@h      
    ori %r5, %r5, _Z16DoPreferenceSortP14CInventoryView@l
    lwz %r2, 0x4(%r5)
    bl ._Z16DoPreferenceSortP14CInventoryView
    ld %r2, 0x28(%r1)

    ba 0x000c2cf4

.global _global_icon_size_hook
_global_icon_size_hook:
    # Handle previous check for pod types
    bne %cr7, UseLargeIcons

    # All tools should use standard size icons
    lwz %r9, 0x48(%r27)
    cmpwi %cr7, %r9, 0
    bne %cr7, UseStandardIcons

    # Outfits should use large icons
    lwz %r0, 0x44(%r27)
    rlwinm %r9, %r0, 0x0, 0x0, 0x0
    cmpwi %cr7, %r9, 0
    bne %cr7, UseLargeIcons
    
    # User Stickers
    lwz %r0, 0x40(%r27)
    # rlwinm %r9, %r0, 0x0, 0x15, 0x15
    # cmpwi %cr7, %r9, 0
    # bne %cr7, UseLargeIcons

    # User Objects
    rlwinm %r9, %r0, 0x0, 0x18, 0x18
    cmpwi %cr7, %r9, 0
    bne %cr7, UseLargeIcons

UseStandardIcons:
    ba 0x0037f910
UseLargeIcons:
    ba 0x0037fc38

.global _custom_tool_type_hook
_custom_tool_type_hook:
    rldicl %r3, %r31, 0x0, 0x20
    mr %r4, %r10

    std %r2, 0x28(%r1)
    lis %r5, _Z20HandleCustomToolTypeP7CPoppet9EToolType@h      
    ori %r5, %r5, _Z20HandleCustomToolTypeP7CPoppet9EToolType@l
    lwz %r2, 0x4(%r5)
    bl ._Z20HandleCustomToolTypeP7CPoppet9EToolType
    ld %r2, 0x28(%r1)

    ba 0x003466d4

.global _custom_poppet_message_hook
_custom_poppet_message_hook:
    mr %r3, %r28
    mr %r4, %r29

    std %r2, 0x28(%r1)
    lis %r5, _Z25HandleCustomPoppetMessageP7CPoppet18EPoppetMessageType@h      
    ori %r5, %r5, _Z25HandleCustomPoppetMessageP7CPoppet18EPoppetMessageType@l
    lwz %r2, 0x4(%r5)
    bl ._Z25HandleCustomPoppetMessageP7CPoppet18EPoppetMessageType
    ld %r2, 0x28(%r1)

    ba 0x0034fa40

.global _custom_pick_object_action_hook
_custom_pick_object_action_hook:
    cmpwi %cr7, %r3, 0x30
    bne %cr7, NotACustomSubMode

    mr %r3, %r26
    mr %r4, %r27

    std %r2, 0x28(%r1)
    lis %r5, _Z12SetUnphysicsP7CPoppetP6CThing@h      
    ori %r5, %r5, _Z12SetUnphysicsP7CPoppetP6CThing@l
    lwz %r2, 0x4(%r5)
    bl ._Z12SetUnphysicsP7CPoppetP6CThing
    ld %r2, 0x28(%r1)

    li %r9, 0x0
    ba 0x00351738

NotACustomSubMode:
    cmpwi %cr7, %r3, 0x1c
    ba 0x003516e4

.global _fixup_custom_pick_object_select_hook
_fixup_custom_pick_object_select_hook:
    cmpwi %cr7, %r27, 0x30
    beq %cr7, EarlyReturn
    cmpwi %cr7, %r27, 0x1c
    ba 0x00352104

EarlyReturn:
    ba 0x00352028

.global _fady_thing_hook
_fady_thing_hook:
    stw %r3, 0x28(%r1)

    lwz %r3, 0x178(%r27)

    stw %r2, 0x2c(%r1)
    lis %r2, _Z11IsThingFadyP6CThing@h      
    ori %r2, %r2, _Z11IsThingFadyP6CThing@l
    lwz %r2, 0x4(%r2)
    bl ._Z11IsThingFadyP6CThing
    lwz %r2, 0x2c(%r1)

    neg %r3, %r3
    rldicl %r11, %r3, 0x1, 0x3f
    
    lwz %r3,0x28(%r1)
    ba 0x001f0b5c

.global _custom_item_grid_hook
_custom_item_grid_hook:
    rldicl %r31, %r31, 0x0, 0x20
    
    lwz %r4, 0x68(%r31)
    rlwinm %r4, %r4, 0x0, 0x9, 0x9
    cmpwi %cr7, %r4, 0
    
    vsldoi %v2, %v31, %v31, 0x0
    rldicl %r10, %r3, 0x0, 0x38
    lbz %r9, 0x16(%r28)
    mr %r3, %r26
    mr %r4, %r22
    mr %r5, %r31
    
    beq %cr7, UseDefaultItemGridButton
    
    std %r2, 0x28(%r1)
    lis %r2, _Z28DoInventorySoundObjectButtonP12CPoppetChildyP14CInventoryItemN10Vectormath3Aos7Vector4Ebb@h      
    ori %r2, %r2, _Z28DoInventorySoundObjectButtonP12CPoppetChildyP14CInventoryItemN10Vectormath3Aos7Vector4Ebb@l
    lwz %r2, 0x4(%r2)
    bl ._Z28DoInventorySoundObjectButtonP12CPoppetChildyP14CInventoryItemN10Vectormath3Aos7Vector4Ebb
    ld %r2, 0x28(%r1)

    ba 0x003801a0
UseDefaultItemGridButton:
    ba 0x0038019c

.global _custom_event_projects_hook
_custom_event_projects_hook:
    stw %r2, 0x2c(%r1)
    lis %r2, _Z20LoadAllEventProjectsv@h      
    ori %r2, %r2, _Z20LoadAllEventProjectsv@l
    lwz %r2, 0x4(%r2)
    bl ._Z20LoadAllEventProjectsv
    lwz %r2, 0x2c(%r1)

    ba 0x001a4acc

