#ifndef SCRIPT_FUNCTION_H
#define SCRIPT_FUNCTION_H

#include <set>

#include <mem_stl_buckets.h>
#include <refcount.h>

#include "ResourceScript.h"
#include "vm/VMTypes.h"

namespace NVirtualMachine {
    class CScriptFunctionBinding {
    public:
        CScriptFunctionBinding();
    public:
        void Clear();
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

#endif // SCRIPT_FUNCTION_H