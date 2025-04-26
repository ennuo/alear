.include "asm/macros/fnptr.s"

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

.global _run_frame_hook
_run_frame_hook:
    stw %r2, 0x2c(%r1)
    lis %r2, _Z10OnRunFrameP5RGameRK10CRawVectorI13CNetworkInput12CAllocatorMMEb@h      
    ori %r2, %r2, _Z10OnRunFrameP5RGameRK10CRawVectorI13CNetworkInput12CAllocatorMMEb@l
    lwz %r2, 0x4(%r2)
    bl ._Z10OnRunFrameP5RGameRK10CRawVectorI13CNetworkInput12CAllocatorMMEb
    lwz %r2, 0x2c(%r1)

    cmpwi %cr7, %r3, 0x0
    beq %cr7, ExitFrameHook
    
    # continue executing frame
    ba 0x000b1328

ExitFrameHook:
    # branch to function epilogue
    li %r0, 0xe0
    ba 0x000b1990

.global _base_profile_load_hook
_base_profile_load_hook:
    std %r2, 0x28(%r1)
    lis %r2, _Z25OnBaseProfileLoadFinishedP12CBaseProfile@h      
    ori %r2, %r2, _Z25OnBaseProfileLoadFinishedP12CBaseProfile@l
    lwz %r2, 0x4(%r2)
    bl ._Z25OnBaseProfileLoadFinishedP12CBaseProfile
    ld %r2, 0x28(%r1)

    mr %r3, %r31
    addi %r8, %r3, 0x74
    lwz %r0, 0x4(%r8)
    ba 0x000b2694

.global _gmat_player_colour_hook
_gmat_player_colour_hook:
    lwz %r0, 0x0(%r29)
    cmpwi %cr7, %r0, 2
    bne %cr7, NoUserDefinedColour

    li %r0, 1
    stb %r0, 0xea(%r28)
NoUserDefinedColour:
    addi %r29, %r29, 0x2c
    ba 0x00717074


.global _on_reflect_load_thing_hook
_on_reflect_load_thing_hook:
    mr %r3, %r26

    std %r2, 0x28(%r1)
    lis %r2, _ZN6CThing6OnLoadEv@h      
    ori %r2, %r2, _ZN6CThing6OnLoadEv@l
    lwz %r2, 0x4(%r2)
    bl ._ZN6CThing6OnLoadEv
    ld %r2, 0x28(%r1)

    ba 0x00770958

.global _on_reflect_start_save_thing_hook
_on_reflect_start_save_thing_hook:
    mr %r3, %r28

    std %r2, 0x28(%r1)
    lis %r2, _ZN6CThing11OnStartSaveEv@h      
    ori %r2, %r2, _ZN6CThing11OnStartSaveEv@l
    lwz %r2, 0x4(%r2)
    bl ._ZN6CThing11OnStartSaveEv
    ld %r2, 0x28(%r1)

    mr %r3, %r31
    addi %r4, %r28, 0xc
    rldicl %r4, %r4, 0x0, 0x20
    lwz %r31, 0x14(%r27)
    ba 0x0076cf30

.global _on_reflect_finish_save_thing_hook
_on_reflect_finish_save_thing_hook:
    mr %r26, %r3
    mr %r3, %r28

    std %r2, 0x28(%r1)
    lis %r2, _ZN6CThing12OnFinishSaveEv@h      
    ori %r2, %r2, _ZN6CThing12OnFinishSaveEv@l
    lwz %r2, 0x4(%r2)
    bl ._ZN6CThing12OnFinishSaveEv
    ld %r2, 0x28(%r1)

    mr %r3, %r26
    ba 0x0076cf38

create_hook on_fixup_thing_hook, 0x003c4228
    mr %r3, %r28
    call _ZN6CThing7OnFixupEv
    rldicl %r9, %r28, 0x0, 0x20
    ret

.global _custom_gooey_network_action_hook
_custom_gooey_network_action_hook:
    mr %r3, %r31

    std %r2, 0x28(%r1)
    lis %r2, _Z23DoNetworkActionResponseR19CMessageGooeyAction@h      
    ori %r2, %r2, _Z23DoNetworkActionResponseR19CMessageGooeyAction@l
    lwz %r2, 0x4(%r2)
    bl ._Z23DoNetworkActionResponseR19CMessageGooeyAction
    ld %r2, 0x28(%r1)

    ba 0x004372a8

.global _get_frizzlefry_shader_hook
_get_frizzlefry_shader_hook:
    std %r2, 0x28(%r1)
    lis %r2, _Z21GetIceFirePixelShaderv@h      
    ori %r2, %r2, _Z21GetIceFirePixelShaderv@l
    lwz %r2, 0x4(%r2)
    bl ._Z21GetIceFirePixelShaderv
    ld %r2, 0x28(%r1)

    mr %r31, %r3
    ba 0x00417180

.global _is_frizzlefry_lethal_hook
_is_frizzlefry_lethal_hook:
    lwz %r3, 0x70(%r31)

    std %r2, 0x28(%r1)
    lis %r2, _Z18IsFrizzleFryLethal11ELethalType@h      
    ori %r2, %r2, _Z18IsFrizzleFryLethal11ELethalType@l
    lwz %r2, 0x4(%r2)
    bl ._Z18IsFrizzleFryLethal11ELethalType
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 1
    beq %cr7, IsFrizzleFryHook
    ba 0x004172ac
IsFrizzleFryHook:
    ba 0x004172bc


.global _track_frizzlefry_lethal_hook
_track_frizzlefry_lethal_hook:
.set TrackAudioSfx, 0x005bdd24
    std %r2, 0x28(%r1)
    lis %r2, _Z23GetFrizzleFryLethalTypev@h      
    ori %r2, %r2, _Z23GetFrizzleFryLethalTypev@l
    lwz %r2, 0x4(%r2)
    bl ._Z23GetFrizzleFryLethalTypev
    ld %r2, 0x28(%r1)

    mr %r4, %r30

    bla TrackAudioSfx
    ld %r2, 0x28(%r1)

    ba 0x00417460

.global _draw_icey_things_hook
_draw_icey_things_hook:
.set DrawFireyThings, 0x005b88a4
    bla DrawFireyThings
    ld %r2, 0x28(%r1)
    bla DrawFireyThings
    ld %r2, 0x28(%r1)
    ba 0x1dd974

.global _set_danger_type_ice_timer_hook
_set_danger_type_ice_timer_hook:
    mr %r3, %r26

    std %r2, 0x28(%r1)
    lis %r2, _Z17ResetLethalTimersP6PShape@h      
    ori %r2, %r2, _Z17ResetLethalTimersP6PShape@l
    lwz %r2, 0x4(%r2)
    bl ._Z17ResetLethalTimersP6PShape
    ld %r2, 0x28(%r1)

    ba 0x00348138

.global _draw_mesh_boundary_ice_hook
_draw_mesh_boundary_ice_hook:
    cmpwi %cr7, %r9, 3
    beq %cr7, IceHook
    cmpwi %cr7, %r9, 1
    beq %cr7, FireHook
    ba 0x001c5188
IceHook:
    lwz %r9, -0x10c(%r2)
    li %r0, 0x0
    li %r11, 3
    stw %r0, 0x0(%r9)
    ba 0x001c519c
FireHook:
    lwz %r9, -0x10c(%r2)
    li %r0, 0x0
    li %r11, 1
    stw %r0, 0x0(%r9)
    ba 0x001c519c

    # ba 0x001c5354

.global _anim_update_ice_hook
_anim_update_ice_hook:
    mr %r3, %r31

    std %r2, 0x28(%r1)
    lis %r2, _ZN12CSackBoyAnim11DoThawAnimsEv@h      
    ori %r2, %r2, _ZN12CSackBoyAnim11DoThawAnimsEv@l
    lwz %r2, 0x4(%r2)
    bl ._ZN12CSackBoyAnim11DoThawAnimsEv
    ld %r2, 0x28(%r1)

    li %r0, 0
    ba 0x000fecf4

.global _anim_choose_idle_ice_hook
_anim_choose_idle_ice_hook:
    mr %r3, %r31
    mr %r4, %r30

    std %r2, 0x28(%r1)
    lis %r2, _ZN12CSackBoyAnim21UpdateFreezeIdleStateEi@h      
    ori %r2, %r2, _ZN12CSackBoyAnim21UpdateFreezeIdleStateEi@l
    lwz %r2, 0x4(%r2)
    bl ._ZN12CSackBoyAnim21UpdateFreezeIdleStateEi
    ld %r2, 0x28(%r1)

    mr %r30, %r3
    extsw %r26, %r30

    lwz %r0, 0x150(%r31)
    ba 0x000f6570

.global _death_anim_ice_hook
_death_anim_ice_hook:
    cmpwi %cr7, %r30, 0x3
    bne %cr7, DoElectricCheck

    li %r25, 0x0 # loop?
    lwz %r26, 0xCF0(%r27) # anim
    li %r28, 0 # root bone?
    lwz %r31, 0x598(%r29)
    ba 0x000f23d0
DoElectricCheck:
    cmpwi %cr7, %r30, 0x2
    ba 0x000f2398

.global _sackboy_anim_late_update_hook
_sackboy_anim_late_update_hook:
    mr %r3, %r29

    std %r2, 0x28(%r1)
    lis %r2, _ZN12CSackBoyAnim21UpdateFreezeIdleStateEi@h      
    ori %r2, %r2, _ZN12CSackBoyAnim21UpdateFreezeIdleStateEi@l
    lwz %r2, 0x4(%r2)
    bl ._ZN12CSackBoyAnim21UpdateFreezeIdleStateEi
    ld %r2, 0x28(%r1)

    li %r0, 0xe0
    ba 0x000feba8

.global _sackboy_anim_ice_ik_hook
_sackboy_anim_ice_ik_hook:
    lwz %r0, 0xcf4(%r31)
    cmpwi %cr7, %r0, -1
    bne %cr7, DisableIK

    lwz %r0, 0xcd4(%r31)
    cmpwi %cr7, %r0, -1
    bne %cr7, DisableIK

EnableIK:
    ba 0x000fdec4
DisableIK:
    ba 0x000fdc58

.global _sackboy_ground_distance_ice_hook
_sackboy_ground_distance_ice_hook:
    lwz %r11, 0x934(%r31)
    cmpwi %cr7, %r11, 13
    beq %cr7, IsNonJumpableLethal

    cmpwi %cr7, %r0, 0x0
    beq %cr7, IsJumpableLethal
    cmpwi %cr7, %r0, 0x3
    beq %cr7, IsJumpableLethal
    cmpwi %cr7, %r0, 0x4
    beq %cr7, IsJumpableLethal
    
    # Invincible State
    cmpwi %cr7, %r11, 15
    #cmpwi %cr7, %r0, 0x2
    beq %cr7, IsJumpableLethal
    
    #lwz %r13, 0x70(%r36)
    # Invincible and body is electric
    #cmpwi %cr7, %r11, 15
    #cmpwi %cr7, %r13, 0x2
    #cmpwi %cr7, %r0, 0x2
    #beq %cr7, IsJumpableLethal

IsNonJumpableLethal:
    ba 0x00040904
IsJumpableLethal:
    ba 0x0004091c

# doesn't work, so sad!
/*
.global _water_jumping_diver_suit_hook
_water_jumping_diver_suit_hook:
    lwz %r11, 0x934(%r31)
    cmpwi %cr7, %r11, 0xa
    beq %cr7, UpdateJumpingIsSwimming

    lbz %r0, 0x40(%r9)
    ba 0x00064c0c
    
UpdateJumpingIsSwimming:
    ba 0x00064cdc    
*/

# doesn't work, so sad!
/*
.global _water_boost_disable_swimming_fins_hook
_water_boost_disable_swimming_fins_hook:
    lwz %r11, 0x934(%r3)
    cmpwi %cr7, %r11, 0x11
    beq %cr7, SwimStrokeInput

    lwz %r3, 0x24(%r9)
    ba 0x000372ac
    
SwimStrokeInput:
    ba 0x00037320
*/

# Load list of guids from txt file
.global _can_scale_guid_list_hook
_can_scale_guid_list_hook:
.set ExitCanScaleHook_DisableScaling, 0x0036ae18
.set ExitCanScaleHook_AllowScaling, 0x0036ae4c
    lwz %r3, 0x34(%r9)

    # Function stores whether or not we can currently scale this object
    # in %r10 because it knows the function calls in the block don't use this 
    # volatile register, so we'll just temporarily store it in the stack space
    # used for the tint color.
    std %r10, 0xc0(%r1)

    call _Z12CanScaleMesh5CGUID

    # Restore the last state of the variable keeping track of
    # whether or not we can stale now that we've existed our function
    # with possible side effects.
    ld %r10, 0xc0(%r1)
    
    cmpwi %cr7, %r3, 1
    bne %cr7, DisableScalingHook

    ba ExitCanScaleHook_AllowScaling
DisableScalingHook:
    ba ExitCanScaleHook_DisableScaling

/*
# Prevent jetpack from being collected if frozen
# doesn't work, so sad!
.global _set_jetpack_tether_is_frozen_hook
_set_jetpack_tether_is_frozen_hook:
    lwz %r11, 0x934(%r31)
    cmpwi %cr7, %r11, 0xb
    beq %cr7, JetpackTetherNull

    lwz %r0, 0x24(%r3)
    ba 0x0040b694
    
JetpackTetherNull:
    ba 0x0040b6a8
*/

.global _hack_gather_character_settings_hook
_hack_gather_character_settings_hook:
    mr %r3, %r29
    mr %r4, %r28

    std %r2, 0x28(%r1)
    lis %r2, _Z35HackSerializeExtraCharacterSettingsR16CGatherVariablesR18RCharacterSettings@h      
    ori %r2, %r2, _Z35HackSerializeExtraCharacterSettingsR16CGatherVariablesR18RCharacterSettings@l
    lwz %r2, 0x4(%r2)
    bl ._Z35HackSerializeExtraCharacterSettingsR16CGatherVariablesR18RCharacterSettings
    ld %r2, 0x28(%r1)

    ba 0x006e00fc

.global _get_outline_guid_hook
_get_outline_guid_hook:

    # This function doesn't actually have a stack,
    # so we'll have to backup some registers ourself
    # for the function call.

    stdu %r1, -0x100(%r1)

    # Not sure what registers are actually important,
    # so I'll just store all the registers.
    # Not even going to bother with vector registers since they should all be
    # preserved since this function doesn't deal with any vector operations.
    
    std %r4, 0x30(%r1)
    std %r5, 0x38(%r1)
    std %r6, 0x40(%r1)
    std %r7, 0x48(%r1)
    std %r8, 0x50(%r1)
    std %r9, 0x58(%r1)
    std %r10, 0x60(%r1)
    std %r11, 0x68(%r1)
    std %r12, 0x70(%r1)

    # Also make sure to store the link register
    mflr %r0
    std %r0, 0x110(%r1)

    lwz %r3, 0x34(%r9)
    call _Z18GetOutlinePlanGUIDj

    # Destroy our fake function call stack
    # and restore the registers.

    ld %r4, 0x30(%r1)
    ld %r5, 0x38(%r1)
    ld %r6, 0x40(%r1)
    ld %r7, 0x48(%r1)
    ld %r8, 0x50(%r1)
    ld %r9, 0x58(%r1)
    ld %r10, 0x60(%r1)
    ld %r11, 0x68(%r1)
    ld %r12, 0x70(%r1)

    ld %r0, 0x110(%r1)
    mtlr %r0

    addi %r1, %r1, 0x100

    cmpwi %cr7, %r3, -0x2A43
    beq %cr7, MouthVectors
    cmpwi %cr7, %r3, -0x1395
    beq %cr7, EyeVectors
    b NoWeirdVectorAssignmentExit

MouthVectors:
    ba 0x00359464
EyeVectors:
    ba 0x00359508
NoWeirdVectorAssignmentExit:
    ba 0x0035910c

create_hook render_mesh_setup_rendering_hook, 0x0003ecb8
    mr %r3, %r31
    call _ZNK11PRenderMesh14SetupRenderingEv
    ret

