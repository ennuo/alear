#ifndef NATIVE_FUNCTION_CRACKER_H
#define NATIVE_FUNCTION_CRACKER_H

#include <cstddef>



#include "vm/ConvertTypes.h"
#include "vm/ScriptContext.h"

namespace NVirtualMachine
{
    template<typename ReturnType>
    void ConvertReturnValue(void*& vm_addr, CScriptContext* context, ReturnType& native)
{
        SConvertScriptTypes<ReturnType>::NativeToVM
        (
            *((typename SConvertScriptTypes<ReturnType>::VMType*&) vm_addr), 
            context, 
            (typename SConvertScriptTypes<ReturnType>::NativeType&) native
        );
    }

    template <typename ReturnType>
    class CNativeFunction0 {
    public:
        template <ReturnType (&Fn)()>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            ReturnType value = Fn();
            ConvertReturnValue<ReturnType>(ret, context, value);
        }
    };

    template <typename ReturnType, typename Arg1>
    class CNativeFunction1 {
    public:
        template <ReturnType (&Fn)(Arg1)>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            struct CallStruct
            {
                Arg1 a;
            } 
            call;
            
            SConvertScriptTypes<Arg1>::VMToNative(call.a, context, (typename SConvertScriptTypes<Arg1>::VMType&)((CallStruct*)arguments)->a);
            ReturnType value = Fn(call.a);
            ConvertReturnValue<ReturnType>(ret, context, value);
        }
    };

    class CNativeFunction0V {
    public:
        template <void (&Fn)()>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            Fn();
        }
    };

    template <typename Arg1>
    class CNativeFunction1V {
    public:
        template <void (&Fn)(Arg1)>
        static void Call(CScriptContext* context, void* ret, u8* arguments)
        {
            struct CallStruct
            {
                Arg1 a;
            } 
            call;
            
            SConvertScriptTypes<Arg1>::VMToNative(call.a, context, (typename SConvertScriptTypes<Arg1>::VMType&)((CallStruct*)arguments)->a);
            Fn(call.a);

            // how do we calculate argument alignment?

            // ConvertToNative<TArgumentAlignment<0u, Arg1>(*this, CScriptContext*, u8* arguments, TArgumentAlign<0u, Arg1> a)
                // SConvertScriptTypes<Arg1>::VMToNative(Arg1*, CScriptContext* u8* arguments)
            // Fn(arg1)
            // if not void
                // ConvertReturnValue<ReturnType>(void* vm_addr, CScriptContext* context, ReturnType* native)
        }
    };



}


#endif // NATIVE_FUNCTION_CRACKER_H