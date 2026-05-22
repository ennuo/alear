#include <AlearVM.h>
#include <thing.h>
#include <DebugLog.h>
#include <GameUpdateStage.h>
#include <vm/VirtualMachine.h>


using namespace NVirtualMachine;

typedef std::map<const RScript*, CScriptInstance*, std::less<const RScript*>, STLBucketAlloc<std::pair<const RScript*, CScriptInstance*> > > InstanceMap;
InstanceMap gScriptObjectStatics;

static void* g_ElfTocTable[] =
{
    (void*) TOC0,
    (void*) TOC1
};

void Invoke(ExecutionState* state, shkOpd* fn)
{
    ((void(*)(
        int, int, int, int, int, int, int, int,
        float, float, float, float, v4
    )) fn)
    (
        *(u32*)(state->NewArgs + 0x00),
        *(u32*)(state->NewArgs + 0x04),
        *(u32*)(state->NewArgs + 0x08),
        *(u32*)(state->NewArgs + 0x0c),
        *(u32*)(state->NewArgs + 0x10),
        *(u32*)(state->NewArgs + 0x14),
        *(u32*)(state->NewArgs + 0x18),
        *(u32*)(state->NewArgs + 0x1c),
        *(f32*)(state->NewArgs + 0x20),
        *(f32*)(state->NewArgs + 0x24),
        *(f32*)(state->NewArgs + 0x28),
        *(f32*)(state->NewArgs + 0x2c),
        *(v4*) (state->NewArgs + 0x30)
    );
}

template <typename T>
void InvokeAndStore(ExecutionState* state, shkOpd* fn, u8* storage)
{
    T ret = ((T(*)(
        int, int, int, int, int, int, int, int,
        float, float, float, float, v4
    )) fn)
    (
        *(u32*)(state->NewArgs + 0x00),
        *(u32*)(state->NewArgs + 0x04),
        *(u32*)(state->NewArgs + 0x08),
        *(u32*)(state->NewArgs + 0x0c),
        *(u32*)(state->NewArgs + 0x10),
        *(u32*)(state->NewArgs + 0x14),
        *(u32*)(state->NewArgs + 0x18),
        *(u32*)(state->NewArgs + 0x1c),
        *(f32*)(state->NewArgs + 0x20),
        *(f32*)(state->NewArgs + 0x24),
        *(f32*)(state->NewArgs + 0x28),
        *(f32*)(state->NewArgs + 0x2c),
        *(v4*) (state->NewArgs + 0x30)
    );

    *((T*)storage) = ret;
}

void GatherInstanceHints(const RScript* script, u32& instance_size, u32& num_fields)
{
    instance_size = 0;
    num_fields = 0;

    for (u32 i = 0; i < script->GetNumFields(); ++i)
    {
        const CFieldDefinitionRow* field = script->GetField(i);
        if (field->Modifiers.IsSet(MT_NATIVE) || !field->Modifiers.IsSet(MT_STATIC)) continue;
        
        const CTypeReferenceRow* type_ref = script->GetType(field->TypeReferenceIdx);
        u32 type_size = GetTypeSize(type_ref->GetMachineType());

        instance_size = RoundUpPow2(instance_size, type_size) + type_size;
        num_fields += 1;
    }
}

CScriptInstance* GetOrCreateStaticInstance(const RScript* script, bool reload_if_exists = false)
{
    CScriptInstance* old_instance = NULL;

    // We'll basically handle all static members by initializing an instance
    // of the script, but only calling the static initializer.
    InstanceMap::iterator it = gScriptObjectStatics.find(script);
    if (it != gScriptObjectStatics.end())
        old_instance = it->second;

    if (!reload_if_exists)
        return old_instance;
    
    u32 num_static_fields, instance_size;
    GatherInstanceHints(script, instance_size, num_static_fields);

    CScriptInstance* instance = new CScriptInstance((RScript*)script);
    instance->InstanceLayout = new CInstanceLayout(instance_size, num_static_fields);
    const CP<CInstanceLayout>& layout = instance->GetInstanceLayout();

    u32 instance_offset = 0;
    for (u32 i = 0; i < script->GetNumFields(); ++i)
    {
        const CFieldDefinitionRow* field = script->GetField(i);
        if (field->Modifiers.IsSet(MT_NATIVE) || !field->Modifiers.IsSet(MT_STATIC)) continue;
        
        const CTypeReferenceRow* type_ref = script->GetType(field->TypeReferenceIdx);
        u32 type_size = GetTypeSize(type_ref->GetMachineType());

        instance_offset = RoundUpPow2(instance_size, type_size);

        layout->AddField(
            script->LookupStringA(field->NameStringIdx),
            field->Modifiers,
            type_ref->GetFishType(),
            type_ref->GetMachineType(),
            type_ref->DimensionCount,
            type_ref->GetArrayBaseMachineType(),
            field->InstanceOffset,
            field->FieldNameHash
        );

        instance_offset += type_size;
    }

    CRawVector<unsigned char>& members = instance->MemberVariables;
    members.resize(instance_size);
    if (instance_size != 0)
        memset(members.begin(), 0, instance_size);

    gScriptObjectStatics.insert(InstanceMap::value_type(script, instance));
    if (old_instance)
    {
        // copy old data back over
        delete old_instance;
    }
    
    {
        CMainGameStageOverride _stage_override(E_UPDATE_STAGE_OTHER_WORLD);
        CScriptArguments args;
        
        CThing* world = new CThing();
        world->AddPart(PART_TYPE_WORLD);

        NVirtualMachine::ExecuteStatic(world, script->GetGUID(), ".static_init__", args, true, NULL);
        NVirtualMachine::ExecuteStatic(world, script->GetGUID(), ".static_ctor__", args, true, NULL);

        delete world;
    }

    return instance;
}

void EnsureStaticInstanceExists(const RScript* script)
{
    GetOrCreateStaticInstance(script, true);
}

void AlearHandleVM(ExecutionState* state)
{
    Instruction instruction = state->Bytecode[state->PC];
    switch (instruction.Bits & 0xff)
    {
        case IT_EXT_GET_STATIC_MEMBER:
        {
            const CFieldReferenceRow* field = state->Script->GetFieldReference(instruction.GetMember.FieldRef);
            CScriptInstance* instance = GetOrCreateStaticInstance(state->Script);
            memcpy(state->Storage + instruction.GetMember.DstIdx, instance->GetMembers().begin() + field->InstanceOffset, GetTypeSize((EMachineType)instruction.GetMember.Type));
            break;
        }
        case IT_EXT_SET_STATIC_MEMBER:
        {
            const CFieldReferenceRow* field = state->Script->GetFieldReference(instruction.SetMember.FieldRef);
            CScriptInstance* instance = GetOrCreateStaticInstance(state->Script);
            memcpy(instance->GetMembers().begin() + field->InstanceOffset, state->Storage + instruction.SetMember.SrcIdx, GetTypeSize((EMachineType)instruction.SetMember.Type));
            break;
        }
        case IT_EXT_ADDRESS:
        {
            #ifdef SCRIPT_DEBUG
            MMLogCh(DC_SCRIPT_DEBUG, "[IT_EXT_ADDRESS] Getting address of variable at r%d\n", instruction.Unary.DstIdx);
            #endif

            u32* dst = (u32*)(state->Storage + instruction.Unary.DstIdx);
            *dst = (u32)(state->Storage + instruction.Unary.SrcIdx);
            break;
        }
        case IT_EXT_LOAD:
        {
            void* dst = (void*)(state->Storage + instruction.Memory.DstIdx);
            void* ptr = (void*)(*(u32**)(state->Storage + instruction.Memory.SrcIdx));

            #ifdef SCRIPT_DEBUG
            MMLogCh(DC_SCRIPT_DEBUG, "[IT_EXT_LOAD] Loading %s from r%d(%08x) into r%d\n",
                GetTypeName((EMachineType)instruction.Memory.Type),
                instruction.Memory.SrcIdx,
                ptr, 
                instruction.Memory.DstIdx
            );
            #endif

            memcpy(dst, ptr, GetTypeSize((EMachineType)instruction.Memory.Type));
            break;
        }
        case IT_EXT_STORE:
        {
            void* dst = (void*)(*(u32**)(state->Storage + instruction.Memory.DstIdx));
            void* ptr = (void*)(state->Storage + instruction.Memory.SrcIdx);

            #ifdef SCRIPT_DEBUG
            MMLogCh(DC_SCRIPT_DEBUG, "[IT_EXT_STORE] Storing %s from r%d to r%d(%08x)\n",
                GetTypeName((EMachineType)instruction.Memory.Type),
                instruction.Memory.SrcIdx, 
                instruction.Memory.DstIdx,
                dst
            );
            #endif

            memcpy(dst, ptr, GetTypeSize((EMachineType)instruction.Memory.Type));
            break;
        }
        case IT_EXT_INVOKE_CONSTANT:
        {
            #ifdef SCRIPT_DEBUG
            MMLogCh(DC_SCRIPT_DEBUG, "[IT_EXT_INVOKE_CONSTANT] Invoking (%s)(%08x) and storing result in r%d\n",
                GetTypeName((EMachineType) instruction.Invoke.Type),
                instruction.Invoke.SrcOrIdx,
                instruction.Invoke.DstIdx
            );
            #endif

            shkOpd fn = 
            {
                (void*)instruction.Invoke.SrcOrIdx,
                g_ElfTocTable[instruction.Invoke.TocIdx]
            };

            u8* storage = state->Storage + instruction.Invoke.DstIdx;

            switch (instruction.Invoke.Type)
            {
                case VMT_VOID: Invoke(state, &fn); break;
                case VMT_BOOL: InvokeAndStore<bool>(state, &fn, storage); break;
                case VMT_CHAR: InvokeAndStore<tchar_t>(state, &fn, storage); break;
                case VMT_VECTOR4: InvokeAndStore<v4>(state, &fn, storage); break;
                case VMT_S64: InvokeAndStore<u64>(state, &fn, storage); break;
                case VMT_F64: InvokeAndStore<f64>(state, &fn, storage); break;
                default:
                {
                    InvokeAndStore<u32>(state, &fn, storage);
                    break;
                }
            }

            break;
        }
    }
}

static s32 gSwitchTable[NUM_INSTRUCTION_TYPES];
const int g_RetailInstructionCount = IT_ASSERT + 1;

extern "C" void _alearvm_hook_naked();
extern "C" void _on_fixup_script_hook();
void AlearInitVMHook()
{

    MH_PokeMemberHook(0x00098edc, RScript::FixupFieldDefinitions);
    MH_PokeBranch(0x000cc0ac, &_on_fixup_script_hook);

    MH_Poke32(0x00189880, 0x2b850000 + NUM_INSTRUCTION_TYPES /* cmplwi %cr7, %r5, NUM_INSTRUCTION_TYPES */);

    // Going to just switch out the switch case table with our own,
    // first have to read the original one and account for the new offsets.
    const int SWITCH_LABEL = 0x001898a4;
    MH_Read(SWITCH_LABEL, gSwitchTable, g_RetailInstructionCount * sizeof(s32));
    for (int i = 0; i < g_RetailInstructionCount; ++i)
    {
        s32 target = SWITCH_LABEL + gSwitchTable[i] - (u32)gSwitchTable;
        gSwitchTable[i] = target;
    }

    // Now fill in the branches to our own custom handling for the rest of the instruction types...
    u32 addr = (u32)(&_alearvm_hook_naked);
    for (int i = g_RetailInstructionCount; i < NUM_INSTRUCTION_TYPES; ++i)
        gSwitchTable[i] = addr - (u32)gSwitchTable;
    
    // Now switch out the pointer in the TOC
    MH_Poke32(0x00921b20, (u32)gSwitchTable);
}