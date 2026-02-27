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

    template <typename T> 
    T GetValue(char const* member, T default_value)
    {
        CP<CInstanceLayout>& layout = ScriptInstance.InstanceLayout;
        if (!layout) return default_value;
    
        SFieldLayoutDetails* field = layout->LookupField(member);
        if (field == NULL) return default_value;
    
        CRawVector<unsigned char>& data = ScriptInstance.MemberVariables;
        if (data.size() >= field->InstanceOffset + sizeof(T))
            return *(T*)(data.begin() + field->InstanceOffset);
        
        return default_value;
    }
    
    template <typename T> 
    void SetValue(char const* member, T value)
    {
        CP<CInstanceLayout>& layout = ScriptInstance.InstanceLayout;
        if (!layout) return;
    
        SFieldLayoutDetails* field = layout->LookupField(member);
        if (field == NULL) return;
    
        CRawVector<unsigned char>& data = ScriptInstance.MemberVariables;
        if (data.size() >= field->InstanceOffset + sizeof(T))
            *(T*)(data.begin() + field->InstanceOffset) = value;
    }

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