#ifndef RESOURCE_DESCRIPTOR_H
#define RESOURCE_DESCRIPTOR_H


#include "GuidHash.h"
#include "ReflectionVisitable.h"

class RMesh;
class RPlan;
class RScript;
class RTranslationTable;
class RFontFace;

enum EResourceType
{
    RTYPE_INVALID=0,
    RTYPE_TEXTURE=1,
    RTYPE_MESH=2,
    RTYPE_PIXEL_SHADER=3,
    RTYPE_VERTEX_SHADER=4,
    RTYPE_ANIM=5,
    RTYPE_GUID_SUBST=6,
    RTYPE_GFXMATERIAL=7,
    RTYPE_SPU_ELF=8,
    RTYPE_LEVEL=9,
    RTYPE_FILENAME=10,
    RTYPE_SCRIPT=11,
    RTYPE_SETTINGS_CHARACTER=12,
    RTYPE_FILE_OF_BYTES=13,
    RTYPE_SETTINGS_SOFT_PHYS=14,
    RTYPE_FONTFACE=15,
    RTYPE_MATERIAL=16,
    RTYPE_DOWNLOADABLE_CONTENT=17,
    RTYPE_EDITOR_SETTINGS=18,
    RTYPE_JOINT=19,
    RTYPE_GAME_CONSTANTS=20,
    RTYPE_POPPET_SETTINGS=21,
    RTYPE_CACHED_LEVEL_DATA=22,
    RTYPE_SYNCED_PROFILE=23,
    RTYPE_BEVEL=24,
    RTYPE_GAME=25,
    RTYPE_SETTINGS_NETWORK=26,
    RTYPE_PACKS=27,
    RTYPE_BIG_PROFILE=28,
    RTYPE_SLOT_LIST=29,
    RTYPE_TRANSLATION=30,
    RTYPE_PARTICLE_SETTINGS=31,
    RTYPE_LOCAL_PROFILE=32,
    RTYPE_LIMITS_SETTINGS=33,
    RTYPE_PARTICLE_TEMPLATE=34,
    RTYPE_PARTICLE_LIBRARY=35,
    RTYPE_AUDIO_MATERIALS=36,
    RTYPE_SETTINGS_FLUID=37,
    RTYPE_PLAN=38,
    RTYPE_TEXTURE_LIST=39,
    RTYPE_MUSIC_SETTING=40,
    RTYPE_MIXER_SETTINGS=41,
    RTYPE_REPLAY_CONFIG=42,
    RTYPE_PALETTE=43,
    RTYPE_LAST=44
};

/* ResourceDescriptor.h: 33  */
class CResourceDescriptorBase : public CDependencyWalkable {
public:
    inline CResourceDescriptorBase()
    {
        memset(this, 0, sizeof(CResourceDescriptorBase));
        Valid = true;
    }    

    inline CResourceDescriptorBase(EResourceType type, CGUID guid) :
    CDependencyWalkable(), Hash()
    {
        Type = type;
        GUID = guid;
        Valid = true;
    }

protected:
    CGUID GUID;
    CHash Hash;
    u32 Type;
    bool Valid;
};

template<class T> inline EResourceType GetResourceType();
template <> inline EResourceType GetResourceType<RMesh>() { return RTYPE_MESH; }
template <> inline EResourceType GetResourceType<RPlan>() { return RTYPE_PLAN; }
template <> inline EResourceType GetResourceType<RScript>() { return RTYPE_SCRIPT; }
template <> inline EResourceType GetResourceType<RTranslationTable>() { return RTYPE_TRANSLATION; }
template <> inline EResourceType GetResourceType<RFontFace>() { return RTYPE_FONTFACE; }

/* ResourceDescriptor.h: 86 */
template <class T>
class CResourceDescriptor : public CResourceDescriptorBase {
public:
    inline CResourceDescriptor() : CResourceDescriptorBase()
    {
        Type = GetResourceType<T>();
    }

    inline CResourceDescriptor(CGUID guid) : CResourceDescriptorBase(GetResourceType<T>(), guid) {}
};

#endif // RESOURCE_DESCRIPTOR_H