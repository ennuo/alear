#include "Variable.h"
#include <hook.h>

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;

template<typename R>
ReflectReturn Reflect(R& r, CSlapMesh& d)
{
    ReflectReturn ret;
    ADD(Sound);
    ADD(GUID);
    ADD(HorizontalForce);
    ADD(VerticalForce);
    ADD(LeftHand);
    ADD(RightHand);
    ADD(Kill);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CSlapStyles& d)
{
    ReflectReturn ret;
    ADD(Meshes);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CEmote& d)
{
    ReflectReturn ret;
    ADD(Plan);
    ADD(Anim);
    ADD(Sounds);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CEmoteSound& d)
{
    ReflectReturn ret;
    ADD(Frame);
    ADD(Sound);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CEmoteBank& d)
{
    ReflectReturn ret;
    ADD(Emotes);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CAnimStyle& d)
{
    ReflectReturn ret;
    ADD(ID);
    ADD(Gsub);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CStyleBank& d)
{
    ReflectReturn ret;
    ADD(Styles);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CRenderJoint& d)
{
    ReflectReturn ret;
    ADD(Mesh);    
    ADD(InactiveMesh);
    ADD(PatternMesh);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CRenderJoints& d)
{
    ReflectReturn ret;
    ADD(Joints);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CPoppetOutline& d)
{
    ReflectReturn ret;
    ADD(Plan);
    ADD(Mesh);
    return ret;
}

template<typename R>
ReflectReturn Reflect(R& r, CPoppetOutlineConfig& d)
{
    ReflectReturn ret;
    ADD(Outlines);
    return ret;
}

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


template<typename R, typename D>
ReflectReturn Reflect(R& r, CResourceDescriptor<D>& d)
{
    return ReflectDescriptor(r, d, true, false);
}

template ReflectReturn Reflect<CReflectionLoadVector, RTexture>(CReflectionLoadVector& r, CP<RTexture>& d);
template ReflectReturn Reflect<CReflectionLoadVector, RPlan>(CReflectionLoadVector& r, CP<RPlan>& d);
template ReflectReturn Reflect<CReflectionLoadVector, RGfxMaterial>(CReflectionLoadVector& r, CP<RGfxMaterial>& d); 
template ReflectReturn Reflect<CReflectionLoadVector, RMesh>(CReflectionLoadVector& r, CP<RMesh>& d); 

template ReflectReturn Reflect<CReflectionSaveVector, RTexture>(CReflectionSaveVector& r, CP<RTexture>& d); 
template ReflectReturn Reflect<CReflectionSaveVector, RPlan>(CReflectionSaveVector& r, CP<RPlan>& d); 
template ReflectReturn Reflect<CReflectionSaveVector, RGfxMaterial>(CReflectionSaveVector& r, CP<RGfxMaterial>& d); 
template ReflectReturn Reflect<CReflectionSaveVector, RMesh>(CReflectionSaveVector& r, CP<RMesh>& d); 

template ReflectReturn Reflect<CReflectionLoadVector, RTexture>(CReflectionLoadVector& r, CResourceDescriptor<RTexture>& d);
template ReflectReturn Reflect<CReflectionLoadVector, RPlan>(CReflectionLoadVector& r, CResourceDescriptor<RPlan>& d);

template ReflectReturn Reflect<CReflectionSaveVector, RTexture>(CReflectionSaveVector& r, CResourceDescriptor<RTexture>& d);
template ReflectReturn Reflect<CReflectionSaveVector, RPlan>(CReflectionSaveVector& r, CResourceDescriptor<RPlan>& d);


#undef ADD

template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CSlapMesh& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CSlapStyles& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CEmote& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CEmoteBank& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CEmoteSound& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CAnimStyle& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CStyleBank& d);

template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CPoppetOutlineConfig& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CPoppetOutline& d);

template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CRenderJoint& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CRenderJoints& d);

MH_DefineFunc(GatherVariablesLoad, 0x003fb94c, TOC1, ReflectReturn, ByteArray& v, CGatherVariables& variables, bool ignore_head, char* header_4bytes);

MH_DefineFunc(Reflect_CGatherVariables_CThingPtr, 0x006f40b0, TOC1, ReflectReturn, CGatherVariables&, CThingPtr&);
template <>
ReflectReturn Reflect(CGatherVariables& r, CThingPtr& d)
{
    return Reflect_CGatherVariables_CThingPtr(r, d);
}