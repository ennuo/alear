.include "asm/macros/fnptr.s"

.global _raycast_hook
_raycast_hook:
    # Dumb little hack, basically we're just going to copy the
    # raycast for network's port fields into the result raycast,
    # this will obviously only work for local players, fix for
    # networked players later on.
    lwz %r0, 0x18fc(%r9)
    stw %r0, 0x18ac(%r9)
    
    li %r0, 0x40
    ba 0x00110680

.global _render_wire_hook
_render_wire_hook:
    call _Z17CustomRenderWiresv
    ba 0x001df90c
