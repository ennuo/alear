#ifndef SCRIPTED_GOOEY_WRAPPER_H
#define SCRIPTED_GOOEY_WRAPPER_H

#include <refcount.h>

#include "ResourceScript.h"
#include "thing.h"
#include "gooey/GooeyNodeManager.h"
#include "scriptobjects/ScriptObjectCustom.h"


class CScriptedGooeyWrapper;
typedef CScriptObjectCustom<CScriptedGooeyWrapper> CScriptObjectGooey;

class CScriptedGooeyWrapper {
public:
    CThingPtr World;
    CP<RScript> Script;
    CScriptObjectGooey* Instance;
    CGooeyNodeManager* Manager;
};

#endif // SCRIPTED_GOOEY_WRAPPER_H