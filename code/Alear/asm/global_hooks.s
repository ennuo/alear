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
