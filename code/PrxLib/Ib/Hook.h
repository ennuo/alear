#pragma once

typedef uint64_t HookHandle;

#define INVALID_HOOK_HANDLE (~0ull)
#define TOC0 (0x009230b8)
#define TOC1 (0x00932B50)

#define PORT_BIND_TEXTURE (0x001cadc8)

struct v4hack { v4 V; };

template<typename T> 
uintptr_t GetFunctionPointer(T fn) 
{ 
    union 
    { 
        uintptr_t opd; 
        T t; 
    } data; 

    data.t = fn; 

    return data.opd; 
} 

struct StaticDetourHandle;
extern StaticDetourHandle* gStaticDetourHead;

struct DetourHandle
{
    inline DetourHandle(const char* name, void* original, void* function) :
    Name(name), Function(function), Original(original), Next()
    {

    }

    const char* Name;
    void* Function;
    void* Original;
    DetourHandle* Next;
};

struct StaticDetourHandle : public DetourHandle
{
    inline StaticDetourHandle(const char* name, void* original, void* function) :
    DetourHandle(name, original, function), PendingInit()
    {
        PendingInit = gStaticDetourHead;
        gStaticDetourHead = this;
    }

    StaticDetourHandle* PendingInit;
};

struct DetourData
{
    uintptr_t Address;
    DetourHandle* First;
};

struct Opd
{
    void* Function;
    void* TOC;
};

#define Ib_Detour(name, display_name, return_type, ...) \
namespace __HOOK_ENCLOSURE__ ## name { \
static Opd OriginalFunction = { (void*)(PORT_##name ## _ADDRESS), (void*)(PORT_##name ## _TOC) }; \
extern return_type Hook(__VA_ARGS__); \
static StaticDetourHandle Handle(display_name, (void*)&OriginalFunction, (void*)&Hook); \
typedef return_type (*FunctionType)(__VA_ARGS__); \
typedef return_type ReturnType; \
return_type Hook(__VA_ARGS__)

#define Ib_Continue(...) ({ \
    (Handle.Next == NULL) ? \
        (*(FunctionType*)&Handle.Original)(__VA_ARGS__) \
    : \
        (*(FunctionType*)&Handle.Next->Function)(__VA_ARGS__); \
})

#define Ib_EndDetour }


#define Ib_DefineFunc(name, address, toc, type, ...) \
static Opd _##name = { (void*)(address), (void*)(toc) }; \
type(*name)(__VA_ARGS__) = (type (*)(__VA_ARGS__))&_##name;

#define Ib_DefinePort(name, type, ...) \
static Opd _##name = { (void*)(PORT_##name ## _ADDRESS), (void*)(PORT_##name ## _TOC) }; \
type(*name)(__VA_ARGS__) = (type (*)(__VA_ARGS__))&_##name;

#define Ib_DeclarePort(name, type, ...) extern type (*name)(__VA_ARGS__)

#define Ib_PokeHook(address, function) Ib::Hook((uintptr_t)address, (const void*)&function)
#define Ib_ReplacePort(name, function) \
    printf("ib: replacing port %s -> %s\n", #name, #function); \
    Ib_PokeHook(PORT_##name ## _ADDRESS, function);
#define Ib_PokeCall(address, function) Ib::ReplaceCall((uintptr_t)address, (void*)&function)
#define Ib_PokeBranch(address, destination) Ib::PokeBranch((uintptr_t)address, (uintptr_t)destination)

namespace Ib
{
    template <typename T>
    class VecAllBase {
    public:
        virtual T* Allocate(uint32_t count = 1) = 0;
        virtual uint32_t Size() const = 0;
        virtual T* Get(uint32_t index) const = 0;
    };

    template <typename T>
    struct DirtyAllocator : public VecAllBase<T>
    {
        T* data;
        uint32_t size;
        uint32_t max_size;

        inline DirtyAllocator(const char* name, T* p, uint32_t capacity) :
        data(p), size(0), max_size(capacity)
        {
            printf("ib: dirty alloc %s data=0x%08x, capacity=%d (raw_size=%d)\n", name, data, (int)(capacity / sizeof(T)), capacity);
        }

        inline T* Allocate(uint32_t count)
        {
            if (size + count >= max_size) return 0;
            T* ptr = data + size;
            size += count;
            return ptr;
        }

        inline uint32_t Size() const { return size; }
        inline T* Get(uint32_t i) const { return data + i; }
    };
    
    struct InitArgs
    {
        VecAllBase<char>* ExecutableData;
        VecAllBase<DetourData>* Detours;
        
        // Pointer to an allocated EmulatorWriteCache instance,
        // this is only necessary on RPCS3.
        void* WriteCache;
    };

    void Initialize(InitArgs* args);
    void Close();

    HookHandle Hook(uintptr_t address, const void* func);
    HookHandle ReplaceCall(uintptr_t address, const void* hook);
    void PokeBranch(uintptr_t address, uintptr_t destination);
    
    void Release(HookHandle handle);
}
