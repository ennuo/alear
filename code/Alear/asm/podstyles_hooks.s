.global _setinventorypod_hook
_setinventorypod_hook:
    # Load the subtype
    lwz %r6, 0xb4(%r6)
    cmpwi %cr7, %r6, 0x1
    beq %cr7, _SetInventoryPodController
    cmpwi %cr7, %r6, 0x2
    beq %cr7, _SetInventoryPodMesh
    b _SetInventoryPod

# Branch to our custom handling function
_SetInventoryPodController:
    std %r2, 0x28(%r1)
    lis %r7, _Z25SetInventoryPodControllerPvP6CThingRK23CResourceDescriptorBase@h      
    ori %r7, %r7, _Z25SetInventoryPodControllerPvP6CThingRK23CResourceDescriptorBase@l
    lwz %r2, 0x4(%r7)
    bl ._Z25SetInventoryPodControllerPvP6CThingRK23CResourceDescriptorBase
    ld %r2, 0x28(%r1)

    b _SetInventoryPod_Exit

_SetInventoryPodMesh:
    std %r2, 0x28(%r1)
    lis %r7, _Z19SetInventoryPodMeshPvP6CThingRK23CResourceDescriptorBase@h      
    ori %r7, %r7, _Z19SetInventoryPodMeshPvP6CThingRK23CResourceDescriptorBase@l
    lwz %r2, 0x4(%r7)
    bl ._Z19SetInventoryPodMeshPvP6CThingRK23CResourceDescriptorBase
    ld %r2, 0x28(%r1)

    b _SetInventoryPod_Exit

# This is the usual path this code is meant to take
# so we don't have to do anything
_SetInventoryPod:
    bla 0x00386b9c

_SetInventoryPod_Exit:
    # Return to normal execution
    ba 0x0038adc8