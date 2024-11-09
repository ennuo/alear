.set TextRange_Compare, 0x005b7254

.global _pin_parse_player_metadata_hook
_pin_parse_player_metadata_hook:
    # Check if the current tag in the iterator matches
    # the requested "pins" tag.
    addi %r3, %r1, 0xec
    lwz %r0, 0x174(%r1)
    lwz %r9, 0x178(%r1)
    rldicl %r3, %r3, 0x0, 0x20
    lis %r4, Pins@ha
    addi %r4, %r4, Pins@l
    stw %r0, 0xec(%r1)
    stw %r9, 0xf0(%r1)
    bla TextRange_Compare
    ld %r2, 0x28(%r1)

    cmpwi %cr7, %r3, 0
    bne %cr7, _SkipPlagachefParsing
    lwz %r9, 0x184(%r1)

    # %r21 already has a pointer to our union struct
    rldicl %r4, %r21, 0x0, 0x20

    # Nastiness, get the text range onto the stack for our function call
    lwz %r11, 0x188(%r1)
    rldicr %r0, %r9, 0x20, 0x1f
    stw %r9, 0x134(%r1)
    or %r3, %r11, %r0
    stw %r11, 0x138(%r1)

    # Finally pass execution to our custom handler
    std %r2, 0x28(%r1)
    lis %r7, _Z9ParsePins9TextRangeIcEPj@h      
    ori %r7, %r7, _Z9ParsePins9TextRangeIcEPj@l
    lwz %r2, 0x4(%r7)
    bl ._Z9ParsePins9TextRangeIcEPj
    ld %r2, 0x28(%r1)

    ba 0x00161068
_SkipPlagachefParsing:
    ba 0x00161478

.global _pin_update_http_tasks
_pin_update_http_tasks:
    std %r2, 0x28(%r1)
    lis %r7, _Z17OnUpdateHttpTasksv@h      
    ori %r7, %r7, _Z17OnUpdateHttpTasksv@l
    lwz %r2, 0x4(%r7)
    bl ._Z17OnUpdateHttpTasksv
    ld %r2, 0x28(%r1)

    addi %r3, %r31, 0x3d8
    rldicl %r3, %r3, 0x0, 0x20
    ba 0x001203ac

.data

Pins:   .byte "pins"
        .byte 0
