#ifndef PLAYER_COLOURS_H
#define PLAYER_COLOURS_H

#include "Colour.h"

enum EPlayerColour {
    PLAYER_COLOUR_PRIMARY,
    PLAYER_COLOUR_SECONDARY,
    PLAYER_COLOUR_TERTIARY
};

class CPlayerColours {
public:
    c32 MainColour;
    c32 SubColour1;
    c32 SubColour2;
};


#endif // PLAYER_COLOURS_H