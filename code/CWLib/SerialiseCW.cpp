#include "Variable.h"
#include <hook.h>

template <typename R>
ReflectReturn Reflect(R& r, MMString<char>& d)
{
    return REFLECT_NOT_IMPLEMENTED;
}

template <typename R>
ReflectReturn Reflect(R& r, float& d)
{
    return r.ReadWrite((void*)&d, sizeof(float));
}

template <typename R>
ReflectReturn Reflect(R& r, u32& d)
{
    return r.ReadWrite((void*)&d, sizeof(u32));
}

template <typename R>
ReflectReturn Reflect(R& r, bool& d)
{
    return r.ReadWrite((void*)&d, sizeof(bool));
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

#undef ADD

template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CSlapMesh& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CSlapStyles& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CEmote& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CEmoteBank& d);
template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CEmoteSound& d);

MH_DefineFunc(GatherVariablesLoad, 0x003fb94c, TOC1, ReflectReturn, ByteArray& v, CGatherVariables& variables, bool ignore_head, char* header_4bytes);