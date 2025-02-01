.set DisableExplosionCSG, 0x2d
.set DisableExplosionParticles, 0x2e
.set ExplosionSound, 0x30

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

    lbz %r0, DisableExplosionCSG(%r28)
    cmpwi %cr7, %r0, 0
    beq %cr7, DoExplosionCSG

    bla DeleteThingAndJoints
    ba ExitRadialExplosionHook

DoExplosionCSG:
    bla CSGAndDeleteThing
    ba ExitRadialExplosionHook

.global _explosion_particle_and_sound_hook
_explosion_particle_and_sound_hook:
.set AddExplosionBits, 0x001c23d0
.set ExitExplosionParticleHook, 0x00211774
    lbz %r0, DisableExplosionParticles(%r28)
    cmpwi %cr7, %r0, 0
    beq %cr7, DoExplosionParticles
    b GetExplosiveSound
DoExplosionParticles:
    bla AddExplosionBits
GetExplosiveSound:
    lwz %r4, 0x1880(%r2) # CAudio::gSFX
    li %r0, 0xa0
    lwz %r5, 0x70(%r1)
    
    ba ExitExplosionParticleHook
