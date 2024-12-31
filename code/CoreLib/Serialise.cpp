#include "Serialise.h"
#include "SharedSerialise.h"

#include "cell/DebugLog.h"

#include <hook.h>

CReflectionLoadVector::CReflectionLoadVector(CBaseVector<char>* vec) : // file.h: 183
CReflectionBase(), Vec(vec), Allocated(0), DecryptedVec(), Revision(),
CompressionFlags(DEFAULT_COMPRESS_FLAGS), JobsTag(), LoadPos(), CurJob(), PendingInit(),
Sem(), SPUDecompressAvailable()
{

}


MH_DefineFunc(CReflectionLoadVector_ReadWrite, 0x0058cf14, TOC1, ReflectReturn, CReflectionLoadVector*, void*, int);
MH_DefineFunc(CReflectionSaveVector_ReadWrite, 0x0058c044, TOC1, ReflectReturn, CReflectionSaveVector*, void*, int);

ReflectReturn CReflectionSaveVector::ReadWrite(void* d, int size)
{
    return CReflectionSaveVector_ReadWrite(this, d, size);
}

ReflectReturn CReflectionLoadVector::ReadWrite(void* d, int size) // file.h: 263
{

    // using a function pointer for now because i dont want to deal
    // with the compression handling.
    return CReflectionLoadVector_ReadWrite(this, d, size);

    // if (CurJob == NULL)
    // {
    //     if (LoadPos + size <= Vec->size())
    //     {
    //         memcpy(d, Vec->begin() + LoadPos, size);
    //         LoadPos += size;
    //         return REFLECT_OK;
    //     }

    //     return REFLECT_EXCESSIVE_DATA;
    // }

    // DebugLog("we have a compression job!!! we're going to have an issue!!!\n");
    
    // // not dealing with compression man
    // return REFLECT_NOT_IMPLEMENTED;
}


#include <hook.h>
MH_DefineFunc(ReflectDescriptor_ReflectionLoadVector, 0x0062e564, TOC0, ReflectReturn, CReflectionLoadVector&, CResourceDescriptorBase&, bool, bool);
template <>
ReflectReturn ReflectDescriptor(CReflectionLoadVector& r, CResourceDescriptorBase& d, bool cp, bool type)
{
    return ReflectDescriptor_ReflectionLoadVector(r, d, cp, type);
}

MH_DefineFunc(ReflectDescriptor_ReflectionSaveVector, 0x0062e8e8, TOC0, ReflectReturn, CReflectionSaveVector&, CResourceDescriptorBase&, bool, bool);
template <>
ReflectReturn ReflectDescriptor(CReflectionSaveVector& r, CResourceDescriptorBase& d, bool cp, bool type)
{
    return ReflectDescriptor_ReflectionSaveVector(r, d, cp, type);
}