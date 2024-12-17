.set REFLECT_OK, 0
.set RTYPE_ANIMATED_TEXTURE, 45
.set RTYPE_PINS, 47
.set RTYPE_OUTFIT_LIST, 50

.global _allocatenewresource_rtype_pins
_allocatenewresource_rtype_pins:
    rldicl %r3, %r31, 0x0, 0x20
    
    std %r2, 0x28(%r1)
    lis %r7, _Z20AllocatePinsResource13EResourceFlag@h      
    ori %r7, %r7, _Z20AllocatePinsResource13EResourceFlag@l
    lwz %r2, 0x4(%r7)
    bl ._Z20AllocatePinsResource13EResourceFlag
    ld %r2, 0x28(%r1)
    
    mr %r29, %r3
    mr %r25, %r29
    ba 0x00088bb8

.global _allocatenewresource_rtype_outfit_list
_allocatenewresource_rtype_outfit_list:
    rldicl %r3, %r31, 0x0, 0x20
    
    std %r2, 0x28(%r1)
    lis %r7, _Z26AllocateOutfitListResource13EResourceFlag@h      
    ori %r7, %r7, _Z26AllocateOutfitListResource13EResourceFlag@l
    lwz %r2, 0x4(%r7)
    bl ._Z26AllocateOutfitListResource13EResourceFlag
    ld %r2, 0x28(%r1)
    
    mr %r29, %r3
    mr %r25, %r29
    ba 0x00088bb8

.global _allocatenewresource_rtype_animated_texture
_allocatenewresource_rtype_animated_texture:
    rldicl %r3, %r31, 0x0, 0x20

    std %r2, 0x28(%r1)
    lis %r7, _Z31AllocateAnimatedTextureResource13EResourceFlag@h      
    ori %r7, %r7, _Z31AllocateAnimatedTextureResource13EResourceFlag@l
    lwz %r2, 0x4(%r7)
    bl ._Z31AllocateAnimatedTextureResource13EResourceFlag
    ld %r2, 0x28(%r1)
    
    mr %r29, %r3
    mr %r25, %r29
    ba 0x00088bb8

.global _reflectresource_load_rtype_pins
_reflectresource_load_rtype_pins:
    rldicl %r3, %r3, 0x0, 0x20
    rldicl %r4, %r4, 0x0, 0x20

    std %r2, 0x28(%r1)
    lis %r7, _Z7ReflectI21CReflectionLoadVectorE13ReflectReturnRT_R5RPins@h      
    ori %r7, %r7, _Z7ReflectI21CReflectionLoadVectorE13ReflectReturnRT_R5RPins@l
    lwz %r2, 0x4(%r7)
    bl ._Z7ReflectI21CReflectionLoadVectorE13ReflectReturnRT_R5RPins
    ld %r2, 0x28(%r1)

    mr %r27, %r3
    ba 0x0072735c

.global _reflectresource_load_rtype_outfit_list
_reflectresource_load_rtype_outfit_list:
    rldicl %r3, %r3, 0x0, 0x20
    rldicl %r4, %r4, 0x0, 0x20

    std %r2, 0x28(%r1)
    lis %r7, _Z7ReflectI21CReflectionLoadVectorE13ReflectReturnRT_R11ROutfitList@h      
    ori %r7, %r7, _Z7ReflectI21CReflectionLoadVectorE13ReflectReturnRT_R11ROutfitList@l
    lwz %r2, 0x4(%r7)
    bl ._Z7ReflectI21CReflectionLoadVectorE13ReflectReturnRT_R11ROutfitList
    ld %r2, 0x28(%r1)

    mr %r27, %r3
    ba 0x0072735c

.global _reflectresource_load_rtype_animated_texture
_reflectresource_load_rtype_animated_texture:
    rldicl %r3, %r3, 0x0, 0x20
    rldicl %r4, %r4, 0x0, 0x20

    std %r2, 0x28(%r1)
    lis %r7, _Z7ReflectI21CReflectionLoadVectorE13ReflectReturnRT_R16RAnimatedTexture@h      
    ori %r7, %r7, _Z7ReflectI21CReflectionLoadVectorE13ReflectReturnRT_R16RAnimatedTexture@l
    lwz %r2, 0x4(%r7)
    bl ._Z7ReflectI21CReflectionLoadVectorE13ReflectReturnRT_R16RAnimatedTexture
    ld %r2, 0x28(%r1)

    mr %r27, %r3
    ba 0x0072735c

.global _reflectresource_extra_load_rtype_synced_profile
_reflectresource_extra_load_rtype_synced_profile:
    cmpwi %cr7, %r3, 0x0
    bne %cr7, SyncedProfileLoadExit

    mr %r3, %r31
    mr %r4, %r30
    
    std %r2, 0x28(%r1)
    lis %r7, _Z20OnSerializeExtraDataI21CReflectionLoadVectorE13ReflectReturnRT_R14RSyncedProfile@h      
    ori %r7, %r7, _Z20OnSerializeExtraDataI21CReflectionLoadVectorE13ReflectReturnRT_R14RSyncedProfile@l
    lwz %r2, 0x4(%r7)
    bl ._Z20OnSerializeExtraDataI21CReflectionLoadVectorE13ReflectReturnRT_R14RSyncedProfile
    ld %r2, 0x28(%r1)

SyncedProfileLoadExit:
    mr %r27, %r3
    ba 0x0072735c

.global _reflectresource_extra_save_rtype_synced_profile
_reflectresource_extra_save_rtype_synced_profile:
    cmpwi %cr7, %r3, 0x0
    bne %cr7, SyncedProfileSaveExit

    mr %r3, %r30
    mr %r4, %r28
    
    std %r2, 0x28(%r1)
    lis %r7, _Z20OnSerializeExtraDataI21CReflectionSaveVectorE13ReflectReturnRT_R14RSyncedProfile@h      
    ori %r7, %r7, _Z20OnSerializeExtraDataI21CReflectionSaveVectorE13ReflectReturnRT_R14RSyncedProfile@l
    lwz %r2, 0x4(%r7)
    bl ._Z20OnSerializeExtraDataI21CReflectionSaveVectorE13ReflectReturnRT_R14RSyncedProfile
    ld %r2, 0x28(%r1)

SyncedProfileSaveExit:
    mr %r22, %r3
    ba 0x0072c434

.global _reflectresource_extra_fdepend_rtype_synced_profile
_reflectresource_extra_fdepend_rtype_synced_profile:
    cmpwi %cr7, %r3, 0x0
    bne %cr7, SyncedProfileFindDependenciesExit

    mr %r3, %r30
    mr %r4, %r31
    
    std %r2, 0x28(%r1)
    lis %r7, _Z20OnSerializeExtraDataI27CReflectionFindDependenciesE13ReflectReturnRT_R14RSyncedProfile@h      
    ori %r7, %r7, _Z20OnSerializeExtraDataI27CReflectionFindDependenciesE13ReflectReturnRT_R14RSyncedProfile@l
    lwz %r2, 0x4(%r7)
    bl ._Z20OnSerializeExtraDataI27CReflectionFindDependenciesE13ReflectReturnRT_R14RSyncedProfile
    ld %r2, 0x28(%r1)

SyncedProfileFindDependenciesExit:
    mr %r29, %r3
    ba 0x007259dc

.global _reflectresource_dependinate_ok
_reflectresource_dependinate_ok:
    li %r29, REFLECT_OK
    ba 0x007259dc

.global _get_serialisationtype_hook
_get_serialisationtype_hook:
    cmpwi %cr7, %r3, RTYPE_PINS
    beq %cr7, _SerialisationType_Binary
    cmpwi %cr7, %r3, RTYPE_OUTFIT_LIST
    beq %cr7, _SerialisationType_Binary
    cmpwi %cr7, %r3, RTYPE_ANIMATED_TEXTURE
    beq %cr7, _SerialisationType_Binary
    
    b FallbackGetSerialisationType

_SerialisationType_Binary:
    li %r3, 0
    blr

FallbackGetSerialisationType:
    li %r0, 0x441
    ba 0x00087854

.global _reflectextradata_load
_reflectextradata_load:
    bne %cr4, _SkipExtraSerialization

    mr %r3, %r25
    mr %r4, %r26

    std %r2, 0x28(%r1)
    lis %r7, _Z24ReflectExtraResourceDataI21CReflectionLoadVectorE13ReflectReturnP9CResourceRT_@h      
    ori %r7, %r7, _Z24ReflectExtraResourceDataI21CReflectionLoadVectorE13ReflectReturnP9CResourceRT_@l
    lwz %r2, 0x4(%r7)
    bl ._Z24ReflectExtraResourceDataI21CReflectionLoadVectorE13ReflectReturnP9CResourceRT_
    ld %r2, 0x28(%r1)

    mr %r31, %r3

_SkipExtraSerialization:
    cmpwi %cr4, %r31, 0x0
    ba 0x003c7410

.global _reflectextradata_save
_reflectextradata_save:
    mr %r3, %r20
    mr %r4, %r30
    
    std %r2, 0x28(%r1)
    lis %r7, _Z24ReflectExtraResourceDataI21CReflectionSaveVectorE13ReflectReturnP9CResourceRT_@h      
    ori %r7, %r7, _Z24ReflectExtraResourceDataI21CReflectionSaveVectorE13ReflectReturnP9CResourceRT_@l
    lwz %r2, 0x4(%r7)
    bl ._Z24ReflectExtraResourceDataI21CReflectionSaveVectorE13ReflectReturnP9CResourceRT_
    ld %r2, 0x28(%r1)

    li %r29, 0
    ba 0x003c7ab0

.global _initextradata_localprofile
_initextradata_localprofile:
    mr %r3, %r31

    std %r2, 0x28(%r1)
    lis %r7, _ZN13RLocalProfile19InitializeExtraDataEv@h      
    ori %r7, %r7, _ZN13RLocalProfile19InitializeExtraDataEv@l
    lwz %r2, 0x4(%r7)
    bl ._ZN13RLocalProfile19InitializeExtraDataEv
    ld %r2, 0x28(%r1)

    mr %r9, %r17
    ba 0x000ba1d0

.global _initextradata_syncedprofile
_initextradata_syncedprofile:
    mr %r3, %r27

    std %r2, 0x28(%r1)
    lis %r7, _ZN14RSyncedProfile19InitializeExtraDataEv@h      
    ori %r7, %r7, _ZN14RSyncedProfile19InitializeExtraDataEv@l
    lwz %r2, 0x4(%r7)
    bl ._ZN14RSyncedProfile19InitializeExtraDataEv
    ld %r2, 0x28(%r1)

    addi %r3, %r28, 0x50
    ba 0x000af450
