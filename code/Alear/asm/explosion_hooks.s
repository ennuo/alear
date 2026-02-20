.include "asm/macros/fnptr.s"

.set Flags, 0x2c
.set LethalType, 0x2d
.set CsgType, 0x2e
.set ExplosionStyle, 0x2f

.set ExplosionSound, 0x30

.set EXPLOSIVE_CSG_DISABLED, 0x0
.set EXPLOSIVE_CSG_ADD, 0x1
.set EXPLOSIVE_CSG_CUT, 0x2

.global _radial_explosion_hook
_radial_explosion_hook:
.set CSGAndDeleteThing, 0x00210b58
.set DeleteThingAndJoints, 0x00022784
.set ExitRadialExplosionHook, 0x00211734
    # Explosion sound is going to be overriden
    # by the particle effect matrix, so we'll store the pointer in
    # some safe space on the stack.
    lwz %r0, ExplosionSound(%r28)
    stw %r0, 0x70(%r1)

    lbz %r0, CsgType(%r28)
    cmpwi %cr7, %r0, EXPLOSIVE_CSG_DISABLED
    bne %cr7, DoExplosionCSG

    bla DeleteThingAndJoints
    ba ExitRadialExplosionHook

DoExplosionCSG:
    bla CSGAndDeleteThing
    ba ExitRadialExplosionHook

.global _explosion_particle_and_sound_hook
_explosion_particle_and_sound_hook:
.set AddExplosionBits, 0x001c23d0
.set ExitExplosionParticleHook, 0x00211774
    lbz %r0, Flags(%r28)
    rlwinm %r0, %r0, 0x0, 0x1d, 0x1d
    cmpwi %cr7, %r0, 0
    beq %cr7, DoExplosionParticles
    b GetExplosiveSound
DoExplosionParticles:
    # bla AddExplosionBits
    mr %r4, %r3 # move pos into second argument register
    mr %r3, %r29 # move thing into first
    call _Z19zz_AddExplosionBitsPK6CThingN10Vectormath3Aos7Matrix4E
GetExplosiveSound:
    lwz %r4, 0x1880(%r2) # CAudio::gSFX
    li %r0, 0xa0
    lwz %r5, 0x70(%r1)
    
    ba ExitExplosionParticleHook

.global _explosion_yellowhead_handler_hook
_explosion_yellowhead_handler_hook:
    bge %cr7, PlayerNotHit
    
    mr %r3, %r31
    mr %r4, %r26
    vsldoi %v2, %v13, %v13, 0x0
    call _Z14zz_OnPlayerHitPK6CThingRK13ExplosionInfo2v2

    # If we're in a DEATH state, then skip applying
    # radial force regardless.
    lwz %r3, 0x40(%r31)
    lbz %r0, 0x937(%r3)
    cmpwi %cr7, %r0, 0x5
    beq %cr7, SkipRadialForce
PlayerNotHit:
    lbz %r3, Flags(%r26)
    rlwinm %r3, %r3, 0x0, 0x1f, 0x1f
    cmpwi %cr7, %r3, 0
    bne %cr7, SkipRadialForce
    ba 0x0020f164
SkipRadialForce:
    ba 0x0020f078
