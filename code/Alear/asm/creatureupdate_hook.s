.global _creatureupdate_hook
_creatureupdate_hook:
    # Move the execution state into the first argument register
    mr %r3, %r31

    # Call the extended handler function
    std %r2, 0x28(%r1)
    lis %r5, _Z21OnCreatureStateUpdateR9PCreature@h      
    ori %r5, %r5, _Z21OnCreatureStateUpdateR9PCreature@l
    lwz %r2, 0x4(%r5)
    bl ._Z21OnCreatureStateUpdateR9PCreature
    ld %r2, 0x28(%r1)

    # Break out of the switch case
    lwz %r0, 0x934(%r31)
    ba 0x0007ba8c


.global _creatureupdate_handlecollisions_hook
_creatureupdate_handlecollisions_hook:
    mr %r3, %r31

    std %r2, 0x28(%r1)
    lis %r5, _Z15IsPlayableStateR9PCreature@h      
    ori %r5, %r5, _Z15IsPlayableStateR9PCreature@l
    lwz %r2, 0x4(%r5)
    bl ._Z15IsPlayableStateR9PCreature
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 0x0
    beq %cr7, _CreatureUpdate_Collision_NotPlayable

    ba 0x0007be20

_CreatureUpdate_Collision_NotPlayable:
    ba 0x0007bb50


.global _animupdate_walkable_hook
_animupdate_walkable_hook:
    lbz %r27, 0x937(%r24)
    mr %r3, %r24

    std %r2, 0x28(%r1)
    lis %r5, _Z15IsPlayableStateR9PCreature@h      
    ori %r5, %r5, _Z15IsPlayableStateR9PCreature@l
    lwz %r2, 0x4(%r5)
    bl ._Z15IsPlayableStateR9PCreature
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 0x0
    beq %cr7, _AnimUpdate_Walkable_NotPlayable

    ba 0x000febf8

_AnimUpdate_Walkable_NotPlayable:
    ba 0x000feb2c

.global _updatebodyangle_walkable_hook
_updatebodyangle_walkable_hook:
    lwz %r0, 0x934(%r31)

    cmpwi %cr7, %r0, 0x3
    beq _UpdateBodyAngle_Walkable_Playable

    mr %r3, %r31

    std %r2, 0x28(%r1)
    lis %r5, _Z15IsPlayableStateR9PCreature@h      
    ori %r5, %r5, _Z15IsPlayableStateR9PCreature@l
    lwz %r2, 0x4(%r5)
    bl ._Z15IsPlayableStateR9PCreature
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 0x0
    mr %r3, %r31 # just to prevent issues
    bne %cr7, _UpdateBodyAngle_Walkable_Playable

_UpdateBodyAngle_Walkable_NotPlayable:
    ba 0x00052160
_UpdateBodyAngle_Walkable_Playable:
    ba 0x000520d0

.global _canswim_hook
_canswim_hook:

    # The value in %r9 is used after this check,
    # but we don't know if the function we're calling will
    # set it since it's not a persistent register,
    # so back it up into an unused persistent register.
    mr %r25, %r9

    mr %r3, %r26

    std %r2, 0x28(%r1)
    lis %r5, _Z7CanSwimR9PCreature@h      
    ori %r5, %r5, _Z7CanSwimR9PCreature@l
    lwz %r2, 0x4(%r5)
    bl ._Z7CanSwimR9PCreature
    ld %r2, 0x28(%r1)

    # Restore the registers
    mr %r9, %r25
    mr %r25, %r26

    cmpwi %cr7, %r3, 0x0
    beq %cr7, _CanSwim_NonSwimmableState
    ba 0x00064c84

_CanSwim_NonSwimmableState:
    ba 0x00064bfc


.global _lethaltouch_hook
_lethaltouch_hook:
    mr %r3, %r28
    mr %r4, %r30

    std %r2, 0x28(%r1)
    lis %r5, _Z17IsLethalInstaKillR9PCreature11ELethalType@h      
    ori %r5, %r5, _Z17IsLethalInstaKillR9PCreature11ELethalType@l
    lwz %r2, 0x4(%r5)
    bl ._Z17IsLethalInstaKillR9PCreature11ELethalType
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 0x0
    beq %cr7, _LethalTouch_NotTouchingLethal

    # Branch back to setting the player to dead
    ba 0x00074bd4

_LethalTouch_NotTouchingLethal:
    # Now check if we're touching fire
    cmpwi %cr7, %r30, 0x1
    bne %cr7, _LethalTouch_NotTouchingFire

    mr %r3, %r28

    std %r2, 0x28(%r1)
    lis %r5, _Z16IsAffectedByFireR9PCreature@h      
    ori %r5, %r5, _Z16IsAffectedByFireR9PCreature@l
    lwz %r2, 0x4(%r5)
    bl ._Z16IsAffectedByFireR9PCreature
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 0x0
    beq %cr7, _LethalTouch_NotTouchingFire
    ba 0x00074c70

_LethalTouch_NotTouchingFire:
    ba 0x00074788

.global _creature_statechange_hook
_creature_statechange_hook:
    mr %r5, %r4
    lwz %r4, 0x934(%r3)

    std %r2, 0x28(%r1)
    lis %r6, _Z13OnStateChangeR9PCreature6EStateS1_@h      
    ori %r6, %r6, _Z13OnStateChangeR9PCreature6EStateS1_@l
    lwz %r2, 0x4(%r6)
    bl ._Z13OnStateChangeR9PCreature6EStateS1_
    ld %r2, 0x28(%r1)

    lwz %r0, 0x934(%r27)
    lwz %r3, 0x8(%r27)
    mr %r4, %r30
    ba 0x000719ac

.global _creature_scubagear_equip_hook
_creature_scubagear_equip_hook:
    mr %r3, %r31

    std %r2, 0x28(%r1)
    lis %r5, _Z14IsPowerupStateP9PCreature@h      
    ori %r5, %r5, _Z14IsPowerupStateP9PCreature@l
    lwz %r2, 0x4(%r5)
    bl ._Z14IsPowerupStateP9PCreature
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 0x0
    beq %cr7, _ScubaGear_NotUsingPowerup
    ba 0x00073bc0
_ScubaGear_NotUsingPowerup:
    ba 0x00073b30


