#pragma once

enum EPartType
{
#define PART_MACRO(name, type) type,
    #include <PartList.h>
#undef PART_MACRO
    // this enum controls the size of some data structures and is the last 
    // official part supported by LBP1, so it has to stay here. 
    PART_TYPE_SIZE = 31, 
    
    PART_TYPE_NPC = 31, 
    PART_TYPE_SWITCH_INPUT, 
    PART_TYPE_MICROCHIP, 
    PART_TYPE_MATERIAL_TWEAK, 
    PART_TYPE_MATERIAL_OVERRIDE, 
    PART_TYPE_INSTRUMENT, 
    PART_TYPE_SEQUENCER, 
    PART_TYPE_CONTROLINATOR, 
    PART_TYPE_POPPET_POWERUP, 
    PART_TYPE_POCKET_ITEM, 
    PART_TYPE_TRANSITION, 
    PART_TYPE_FADER, 
    PART_TYPE_ANIMATION_TWEAK, 
    PART_TYPE_WIND_TWEAK, 
    PART_TYPE_POWERUP, 
    PART_TYPE_HUD_ELEM, 
    PART_TYPE_TAG_SYNCHRONIZER, 
    PART_TYPE_WORMHOLE, 
    PART_TYPE_QUEST, 
    PART_TYPE_CONNECTOR_HOOK, 
    PART_TYPE_ATMOSPHERIC_TWEAK, 
    PART_TYPE_STREAMING_DATA, 
    PART_TYPE_STREAMING_HINT 
};
