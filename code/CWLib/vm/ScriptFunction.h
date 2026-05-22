#pragma once

#include <set>
#include <mem_stl_buckets.h>
#include <refcount.h>


#include <refcount.h>
#include <vm/ScriptVariant.h>
#include <vm/ScriptArguments.h>

class RScript;
class PWorld;
class CThing;
class CFunctionDefinitionRow;

class CScriptObjectInstance;

namespace NVirtualMachine 
{
    class CScriptFunctionBinding {
    public:
        CScriptFunctionBinding();
        ~CScriptFunctionBinding();
    public:
        void Set(RScript* script, u32 function_idx);
        void Clear();
        inline bool IsValid() const
        {
            return Script != NULL && FunctionIdx != ~0ul;
        }
    public:
        const CFunctionDefinitionRow* GetFunction() const;
        inline const CP<RScript>& GetScript() const { return Script; }
        inline u32 GetFunctionIndex() const { return FunctionIdx; }
    public:
        bool TryInvokeAsync(CThing*, const CScriptArguments&) const;
        bool TryInvokeSync(CThing*, const CScriptArguments&, CScriptVariant*);
        bool TryInvokeAsync(PWorld*, CScriptObjectInstance*, const CScriptArguments&);
        bool TryInvokeSync(PWorld*, CScriptObjectInstance*, const CScriptArguments&, CScriptVariant*);
    public:
        bool InvokeAsync(CThing*, const CScriptArguments&) const;
        bool InvokeSync(CThing*, const CScriptArguments&, CScriptVariant*);
        bool InvokeAsync(PWorld*, CScriptObjectInstance*, const CScriptArguments&);
        bool InvokeSync(PWorld*, CScriptObjectInstance*, const CScriptArguments&, CScriptVariant*);
    private:
        CP<RScript> Script;
        u32 FunctionIdx;
    };

    class CScriptFunction {
    public:
        bool Rebind();
        bool Bind();
    private:
        CP<RScript> Script;
        CSignature Signature;
        CScriptFunctionBinding Binding;
    };

    typedef std::set<CScriptFunction*, std::less<CScriptFunction*>, STLBucketAlloc<CScriptFunction*> > RegisteredScriptFunctions;
    extern RegisteredScriptFunctions gRegisteredScriptFunctions;

    void UpdateAllScriptFunctionBindings();
}
