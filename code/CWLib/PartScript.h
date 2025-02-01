#ifndef PART_SCRIPT_H
#define PART_SCRIPT_H

#include <refcount.h>

#include "Part.h"
#include "vm/ScriptInstance.h"
#include "vm/ScriptFunction.h"
#include "vm/ScriptArguments.h"

class RScript;

class PScript : public CPart {
public:
    bool InvokeSync(CSignature const& signature, CScriptArguments const& args);
    void SetScript(CP<RScript> const& script);
public:
    CScriptInstance ScriptInstance;
    u64 TimeInUpdate;
    u64 TimeInRender;
    u64 LastTimeInUpdate;
    u64 LastTimeInRender;
    u32 TimesReloaded;
    bool ScriptOnCreateCalled;
    bool BindingsInitialised;
    NVirtualMachine::CScriptFunctionBinding OnRenderBinding;
    NVirtualMachine::CScriptFunctionBinding NeedsPredictionBinding;
};

#endif // PART_SCRIPT_H