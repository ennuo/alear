#include <refcount.h>
#include <GuidHash.h>

#include "poppet/ScriptObjectPoppet.h"
#include "gooey/ScriptedGooeyWrapper.h"
#include "PoppetChild.h"
#include "Camera.h"

class RTexture;
class RScript;


class CPoppetGooey : public CPoppetChild {
enum EPoppetGooeyUpdateState {
    PG_NEED_PLAYER,
    PG_NEED_KICK_SCRIPT,
    PG_OK
};
public:
    CCamera UICamera;
    m44 Trans;
    CP<RTexture> ColourSwatch;
    CGUID NewMusicTarget;
    CScriptedGooeyWrapper ScriptedGooey;
    CP<RScript> ScriptPoppet;
    CScriptObjectPoppet* ScriptPoppetInstance;
    EPoppetGooeyUpdateState ScriptState;
    ScriptObjectUID InstanceUID;
    u64 RequestHighlightUID;
    u32 PageScrollRepeatCounter;
    u32 SwatchIndex;
    u32 LeftStickMoveTime;
    u32 LastInventoryitemUID;
    bool ForceUpdate;
    bool HasMidGameJoin;
    bool UpdateFocusToLastSelectedID;
    bool UpdateFocusToSwitchTrigger;
};