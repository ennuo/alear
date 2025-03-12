#ifndef RESOURCE_DESCRIPTOR_H
#define RESOURCE_DESCRIPTOR_H


#include "GuidHash.h"
#include "ReflectionVisitable.h"

class RMesh;
class RPlan;
class RScript;
class RTranslationTable;
class RFontFace;
class RTexture;
class RMaterial;
class RPins;
class RLocalProfile;
class RGuidSubst;
class RLevel;
class RFileOfBytes;
class ROutfitList;
class RAnim;
class RAnimatedTexture;
class RPixelShader;
class RVertexShader;
class RSyncedProfile;
class RGfxMaterial;
class RCharacterSettings;

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
    //RTYPE_REPLAY_CONFIG=42,
    RTYPE_PALETTE,
    RTYPE_REPLAY_CONFIG, // this is technically supposed to be before RTYPE_PALETTE, but LBP1 is weird

    // these are all LBP2/LBP3 types, just adding them here
    // in case i implement some of them
    RTYPE_STATICMESH,
    RTYPE_ANIMATED_TEXTURE,
    RTYPE_VOIP_RECORDING,
    RTYPE_PINS,
    RTYPE_INSTRUMENT,
    RTYPE_SAMPLE,
    RTYPE_OUTFIT_LIST,
    RTYPE_PAINTBRUSH,
    RTYPE_THING_RECORDING,
    RTYPE_PAINTING,
    RTYPE_QUEST,
    RTYPE_ANIMATION_BANK,
    RTYPE_ANIMATION_SET,
    RTYPE_SKELETON_MAP,
    RTYPE_SKELETON_REGISTRY,
    RTYPE_SKELETON_ANIM_STYLES,
    RTYPE_CROSSPLAY_VITA,
    RTYPE_STREAMING_CHUNK,
    RTYPE_SHARED_ADVENTURE_DATA,
    RTYPE_ADVENTURE_PLAY_PROFILE,
    RTYPE_ANIMATION_MAP,
    RTYPE_CACHED_COSTUME_DATA,
    RTYPE_DATALABELS,
    RTYPE_ADVENTURE_MAPS,
    
    // any custom resource types go here

    // toolkit/workbench
    RTYPE_BONESET = 128,
    RTYPE_SHADER_CACHE,
    RTYPE_SCENEGRAPH,
    RTYPE_TYPE_LIBRARY,

    RTYPE_LAST
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

    inline CResourceDescriptorBase(EResourceType type, CHash& hash) :
    CDependencyWalkable(), Hash(), GUID()
    {
        Type = type;
        Hash = hash;
        Valid = true;
    }
public:
    inline bool operator==(CResourceDescriptorBase const& r) const
    {
        return Type == r.Type && GUID == r.GUID && Hash == r.Hash;
    }

    inline bool operator!=(CResourceDescriptorBase const& r) const
    {
        return Type != r.Type || GUID != r.GUID || Hash != r.Hash;
    }

    inline bool operator<(CResourceDescriptorBase const& r) const
    {
        if (Type != r.Type) return Type < r.Type;

        if (GUID == r.GUID)
            return Hash < r.Hash;

        return GUID < r.GUID;
    }
public:
    inline bool IsValid() const { return Valid; }
    inline bool HasGUID() const { return (bool)GUID; }
    inline CGUID& GetGUID() { return GUID; }
    inline CHash& GetHash() { return Hash; }
    inline EResourceType GetType() const { return (EResourceType)Type; }
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
template <> inline EResourceType GetResourceType<RTexture>() { return RTYPE_TEXTURE; }
template <> inline EResourceType GetResourceType<RPins>() { return RTYPE_PINS; }
template <> inline EResourceType GetResourceType<RGfxMaterial>() { return RTYPE_GFXMATERIAL; }
template <> inline EResourceType GetResourceType<RMaterial>() { return RTYPE_MATERIAL; }
template <> inline EResourceType GetResourceType<RGuidSubst>() { return RTYPE_GUID_SUBST; }
template <> inline EResourceType GetResourceType<RFileOfBytes>() { return RTYPE_FILE_OF_BYTES; }
template <> inline EResourceType GetResourceType<ROutfitList>() { return RTYPE_OUTFIT_LIST; }
template <> inline EResourceType GetResourceType<RAnim>() { return RTYPE_ANIM; }
template <> inline EResourceType GetResourceType<RAnimatedTexture>() { return RTYPE_ANIMATED_TEXTURE; }
template <> inline EResourceType GetResourceType<RLevel>() { return RTYPE_LEVEL; }
template <> inline EResourceType GetResourceType<RVertexShader>() { return RTYPE_VERTEX_SHADER; }
template <> inline EResourceType GetResourceType<RPixelShader>() { return RTYPE_PIXEL_SHADER; }
template <> inline EResourceType GetResourceType<RSyncedProfile>() { return RTYPE_SYNCED_PROFILE; }
template <> inline EResourceType GetResourceType<RLocalProfile>() { return RTYPE_LOCAL_PROFILE; }
template <> inline EResourceType GetResourceType<RCharacterSettings>() { return RTYPE_SETTINGS_CHARACTER; }

/* ResourceDescriptor.h: 86 */
template <class T>
class CResourceDescriptor : public CResourceDescriptorBase {
public:
    inline CResourceDescriptor() : CResourceDescriptorBase()
    {
        Type = GetResourceType<T>();
    }

    inline CResourceDescriptor(CGUID guid) : CResourceDescriptorBase(GetResourceType<T>(), guid) {}
    inline CResourceDescriptor(CHash& hash) : CResourceDescriptorBase(GetResourceType<T>(), hash) {}
};

#endif // RESOURCE_DESCRIPTOR_H