#include "Variable.h"
#include <hook.h>

template <typename R>
ReflectReturn Reflect(R& r, MMString<char>& d)
{
    DebugLog("!! WARNING !! We're trying to reflect a string!\n");
    return REFLECT_NOT_IMPLEMENTED;
}

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;

template<typename R>
ReflectReturn Reflect(R& r, CSlapMesh& d)
{
    ReflectReturn ret;
    ADD(Sound);
    ADD(GUID);
    ADD(Strength);
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

// this is technically meant to be a templated function, but I don't feel like rewriting it right now,
// and we only need the CReflectionLoadVector version

MH_DefineFunc(ReflectResourceCP_CReflectionLoadVector, 0x006e8ce8, TOC1, ReflectReturn, CReflectionLoadVector&, CP<CResource>*, EResourceType);
template<typename R, typename D>
ReflectReturn Reflect(R& r, CP<D>& d)
{
    if (r.IsGatherVariables() || r.GetSaving()) return REFLECT_NOT_IMPLEMENTED;
    ReflectResourceCP_CReflectionLoadVector(r, (CP<CResource>*)&d, GetResourceType<D>());
}


template<typename R, typename D>
ReflectReturn Reflect(R& r, CResourceDescriptor<D>& d)
{
    return ReflectDescriptor(r, d, true, false);
}

template ReflectReturn Reflect<CReflectionLoadVector, RTexture>(CReflectionLoadVector& r, CP<RTexture>& d);
template ReflectReturn Reflect<CReflectionLoadVector, RTexture>(CReflectionLoadVector& r, CResourceDescriptor<RTexture>& d);

template ReflectReturn Reflect<CReflectionLoadVector, RPlan>(CReflectionLoadVector& r, CP<RPlan>& d);
template ReflectReturn Reflect<CReflectionLoadVector, RPlan>(CReflectionLoadVector& r, CResourceDescriptor<RPlan>& d);


#undef ADD

template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CSlapMesh& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CSlapStyles& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CEmote& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CEmoteBank& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CEmoteSound& d);

MH_DefineFunc(GatherVariablesLoad, 0x003fb94c, TOC1, ReflectReturn, ByteArray& v, CGatherVariables& variables, bool ignore_head, char* header_4bytes);