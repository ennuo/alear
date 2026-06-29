#pragma once

#include <vector.h>
#include <ParasiticVector.h>

#include <vm/VMTypes.h>
#include <vm/VMInstruction.h>
#include <vm/ScriptVariant.h>
#include <vm/ScriptArguments.h>
#include <vm/ScriptFunction.h>
#include <vm/ScriptInstance.h>

#include <ResourceScript.h>

#include <hack_thingptr.h>




class ScriptThingUID {
public:
    ScriptThingUID(CThing* thing);
    ScriptThingUID(const ScriptThingUID& uid);
    ScriptThingUID(u32 uid);
public:
    u32 UID;
};

const u32 WORKINGAREA_POOL_SIZE = 4;
const u32 STACK_RESERVE_SIZE = 0x100;
const u16 NO_RETURN_IDX = 0xFFFFu;
const u32 STACK_SIZE = 10000;

struct SWorkingArea
{
    u8 Data[STACK_SIZE];
};

struct ExecutionState
{
    SWorkingArea* WorkingArea;
    const RScript* Script;
    // const CFunctionDefinitionRow* Function;
    const void* Function;
    CParasiticVector<const Instruction> Bytecode;
    u8* Storage;
    u8* NewArgs;
    u32 PC;
    u32 NextPC;
    bool Finished;
    bool CanYield;
};

struct SActivationRecord
{
    inline u32 GetBasePointer() const { return BasePointer; }
    inline u32 GetArg0Pointer() const { return BasePointer; }
    // inline const CFunctionDefinitionRow* GetFunction() const
    // {
    //     return Script->GetFunction(FunctionIdx);
    // }


    const RScript* Script;
    u32 FunctionIdx;
    u32 PC;
    u32 BasePointer;
    EMachineType RVType;
    u32 RVAddress;
    CThingPtr CachedSelfThing;
    bool CachedSelf;
};

enum EScriptExecutionState
{
    SES_OK,
    SES_FINISHED_OK,
    SES_FINISHED_ERRORED
};

class CScriptContext {
public:
    CThingPtr WorldThing;
    CRawVector<unsigned char> SavedStack;
    CVector<SActivationRecord> ActivationRecords;
    ScriptThingUID CachedSelfThingUID;
    CScriptVariant Result;
    EScriptExecutionState State;
    bool YieldASAP;
public:
    void HandleScriptException(const char* msg);
    CThing* LookupThing(ScriptThingUID uid);
};
