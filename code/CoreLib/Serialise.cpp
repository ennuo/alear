#include "Serialise.h"
#include "SharedSerialise.h"
#include "Variable.h"

#include "cell/DebugLog.h"



const u16 CompressionBufferSize = 0x8000;
const u16 CompressionVersion = 1;

u32 gMinRevision = gFormatRevision;

BranchDefine gBranchDefines[] =
{
    { "Leerdammer", gLeerdammerFormatRevision, gLeerdammerBranchDescription }
};

ReflectReturn SRevision::CheckRevision() const
{
    if (Revision > gFormatRevision) return REFLECT_FORMAT_TOO_NEW;
    return IsAfterRevision(gMinSafeRevision) 
        ? REFLECT_OK : REFLECT_FORMAT_TOO_OLD;
}

ReflectReturn SRevision::CheckBranchDescription() const
{
    if (BranchDescription == 0) return REFLECT_OK;

    for (u32 i = 0; i < sizeof(gBranchDefines); ++i)
    {
        BranchDefine& branch = gBranchDefines[i];
        u16 id = branch.BranchDescription >> 16;
        u16 revision = branch.BranchDescription & 0xffff; 
        if (id == GetBranchID() && GetBranchRevision() <= revision)
            return REFLECT_OK;
    }

    return REFLECT_FORMAT_TOO_NEW;
}

CReflectionLoadVector::CReflectionLoadVector(CBaseVector<char>* vec) : // file.h: 183
CReflectionBase(), Vec(vec), Allocated(0), DecryptedVec(), Revision(),
CompressionFlags(DEFAULT_COMPRESS_FLAGS), JobsTag(), LoadPos(), CurJob(), PendingInit(),
Sem(), SPUDecompressAvailable()
{

}

CReflectionSaveVector::CReflectionSaveVector(ByteArray* vec, u32 compression_level) :
CReflectionBase(), Vec(vec), CurJob(NULL), PendingCleanup(NULL), Sem(0, 0),
CompressionFlags(DEFAULT_COMPRESS_FLAGS), EncryptedBlockStart(-1), CompressionLevel(compression_level), JobsTag(0),
SPUCompressAvailable(false)
{
    
}

CReflectionSaveVector::~CReflectionSaveVector()
{
    
}

CReflectionVisitLoad::CReflectionVisitLoad() : Visited()
{

}

MH_DefineFunc(CReflectionLoadVector_ReadWrite, 0x0058cf14, TOC1, ReflectReturn, CReflectionLoadVector*, void*, int);
MH_DefineFunc(CReflectionLoadVector_CleanupDecompression, 0x0058d3d4, TOC1, ReflectReturn, CReflectionLoadVector*);
MH_DefineFunc(CReflectionLoadVector_LoadCompressionData, 0x0058d588, TOC1, ReflectReturn, CReflectionLoadVector*, u32*);


MH_DefineFunc(CReflectionSaveVector_StartCompressing, 0x0058bf7c, TOC1, ReflectReturn, CReflectionSaveVector*);
MH_DefineFunc(CReflectionSaveVector_FinishCompressing, 0x0058c86c, TOC1, ReflectReturn, CReflectionSaveVector*);


MH_DefineFunc(CReflectionSaveVector_ReadWrite, 0x0058c044, TOC1, ReflectReturn, CReflectionSaveVector*, void*, int);


ReflectReturn CReflectionSaveVector::StartCompressing()
{
    return CReflectionSaveVector_StartCompressing(this);
}

ReflectReturn CReflectionSaveVector::FinishCompressing()
{
    return CReflectionSaveVector_FinishCompressing(this);
}

ReflectReturn CReflectionSaveVector::ReadWrite(void* d, int size)
{
    return CReflectionSaveVector_ReadWrite(this, d, size);
}

ReflectReturn CReflectionLoadVector::CleanupDecompression()
{
    return CReflectionLoadVector_CleanupDecompression(this);
}

ReflectReturn CReflectionLoadVector::LoadCompressionData(u32* totalsize)
{
    return CReflectionLoadVector_LoadCompressionData(this, totalsize);
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

MH_DefineFunc(ReflectGP_CGatherVariables, 0x006f2a00, TOC1, ReflectReturn, CGatherVariables& r, CReflectionVisitable*& d, CreateFunc cf, DeleteFunc df, unsigned int size, bool& ad);
template<>
ReflectReturn ReflectGP<CGatherVariables>(CGatherVariables& r, CReflectionVisitable*& d, CreateFunc cf, DeleteFunc df, unsigned int size, bool& add)
{
    return ReflectGP_CGatherVariables(r, d, cf, df, size, add);
}