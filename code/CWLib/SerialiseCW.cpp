


#include <SharedSerialise.h>

// this is technically meant to be a templated function, but I don't feel like rewriting it right now,
// and we only need the CReflectionLoadVector version

MH_DefineFunc(ReflectResourceCP_CReflectionLoadVector, 0x006e8ce8, TOC1, ReflectReturn, CReflectionLoadVector&, CP<CResource>*, EResourceType);
template<typename R, typename D> 
ReflectReturn Reflect(R& r, CP<D>& d) 
{ 
    if (r.IsGatherVariables()) return REFLECT_NOT_IMPLEMENTED; 

    if (r.GetLoading()) 
        return ReflectResourceCP_CReflectionLoadVector((CReflectionLoadVector&)r, (CP<CResource>*)&d, GetResourceType<D>()); 

    return REFLECT_NOT_IMPLEMENTED; 
} 

template ReflectReturn Reflect<CReflectionLoadVector, RTexture>(CReflectionLoadVector& r, CP<RTexture>& d);
template ReflectReturn Reflect<CReflectionLoadVector, RPlan>(CReflectionLoadVector& r, CP<RPlan>& d);
template ReflectReturn Reflect<CReflectionLoadVector, RGfxMaterial>(CReflectionLoadVector& r, CP<RGfxMaterial>& d); 
template ReflectReturn Reflect<CReflectionLoadVector, RMesh>(CReflectionLoadVector& r, CP<RMesh>& d); 
template ReflectReturn Reflect<CReflectionLoadVector, RFluidSettings>(CReflectionLoadVector& r, CP<RFluidSettings>& d);

template ReflectReturn Reflect<CReflectionSaveVector, RTexture>(CReflectionSaveVector& r, CP<RTexture>& d); 
template ReflectReturn Reflect<CReflectionSaveVector, RPlan>(CReflectionSaveVector& r, CP<RPlan>& d); 
template ReflectReturn Reflect<CReflectionSaveVector, RGfxMaterial>(CReflectionSaveVector& r, CP<RGfxMaterial>& d); 
template ReflectReturn Reflect<CReflectionSaveVector, RMesh>(CReflectionSaveVector& r, CP<RMesh>& d); 

template ReflectReturn Reflect<CReflectionLoadVector, RTexture>(CReflectionLoadVector& r, CResourceDescriptor<RTexture>& d);
template ReflectReturn Reflect<CReflectionLoadVector, RPlan>(CReflectionLoadVector& r, CResourceDescriptor<RPlan>& d);

template ReflectReturn Reflect<CReflectionSaveVector, RTexture>(CReflectionSaveVector& r, CResourceDescriptor<RTexture>& d);
template ReflectReturn Reflect<CReflectionSaveVector, RPlan>(CReflectionSaveVector& r, CResourceDescriptor<RPlan>& d);

MH_DefineFunc(GatherVariablesLoad, 0x003fb94c, TOC1, ReflectReturn, ByteArray& v, CGatherVariables& variables, bool ignore_head, char* header_4bytes);
