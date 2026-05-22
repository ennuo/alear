#pragma once

#include <refcount.h>
#include <vector.h>
#include <vm/InstanceLayout.h>
#include <vm/ScriptFunction.h>

#include <utility>

class RScript;
class CInstanceLayout;
class CThing;
class PWorld;

class CScriptInstance {
public:
    CScriptInstance();
    CScriptInstance(const CP<RScript>& script);
    ~CScriptInstance();
public:
    inline const CP<RScript>& GetScript() const { return Script; }
    inline CRawVector<unsigned char>& GetMembers() { return MemberVariables; }
    inline const CRawVector<unsigned char>& GetMembers() const { return MemberVariables; }
    inline const CP<CInstanceLayout>& GetInstanceLayout() const { return InstanceLayout; }
public:
    CP<RScript> Script;
    CP<CInstanceLayout> InstanceLayout;
    CRawVector<unsigned char> MemberVariables;
};
