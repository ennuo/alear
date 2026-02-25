#include "Explode.h"

#include <thing.h>
#include "PartShape.h"

#include <hook.h>

#include <cell/DebugLog.h>

#include <vector.h>
#include <FluidRender.h>
#include <ExplosiveStyles.h>
#include <ResourceSystem.h>
#include <algorithm>

class CCsgState {

};

enum EExplosiveCsgType
{
    EXPLOSIVE_CSG_DISABLED,
    EXPLOSIVE_CSG_ADD,
    EXPLOSIVE_CSG_CUT
};

class CExplosionParams {
public:
    inline CExplosionParams() : Style(), Flags(), CsgType(EXPLOSIVE_CSG_CUT),
    ScorchMarkTexture(), BitsMesh(),FluidSettings(), Sound(), LethalType(LETHAL_FIRE)
    {

    }
public:
    inline CExplosionParams& SetIgnoresPlayer()
    {
        Flags |= kExplosionFlags_DontAffectPlayer;
        return *this;
    }

    inline CExplosionParams& SetDontKillPlayer()
    {
        Flags |= kExplosionFlags_DontKillPlayer;
        return *this;
    }

    inline CExplosionParams& SetLethalType(ELethalType type)
    {
        LethalType = type;
        return *this;
    }

    inline CExplosionParams& SetCSG(int type)
    {
        CsgType = type;
        return *this;
    }

    inline CExplosionParams& SetDisableParticles()
    {
        Flags |= kExplosionFlags_DisableFluidBlobs;
        Flags |= kExplosionFlags_DisableGlassShatter;

        return *this;
    }

    inline CExplosionParams& SetScorchTexture(int key)
    {
        ScorchMarkTexture = LoadResourceByKey<RTexture>(key);
        return *this;
    }

    inline CExplosionParams& SetBitsMesh(int key)
    {
        BitsMesh = LoadResourceByKey<RMesh>(key);
        return *this;
    }

    inline CExplosionParams& SetFluidSettings(int key)
    {
        FluidSettings = LoadResourceByKey<RFluidSettings>(key);
        return *this;
    }

    inline CExplosionParams& SetSound(const char* sound)
    {
        Sound = sound;
        return *this;
    }


public:
    EExplosiveStyle Style;
    int Flags;
    ELethalType LethalType;
    int CsgType;
    StaticCP<RTexture> ScorchMarkTexture;
    StaticCP<RMesh> BitsMesh;
    StaticCP<RFluidSettings> FluidSettings;
    MMString<char> Sound;
};

CExplosionParams gExplosionParams[NUM_EXPLOSIVE_STYLES];
CExplosionParams& GetExplosionParams(int style)
{
    return gExplosionParams[style];
}

CExplosionParams* GetExplosionParams(const CThing* thing)
{
    if (!thing) return NULL;
    PShape* part_shape = thing->GetPShape();
    if (part_shape == NULL) return NULL;

    CP<RMaterial>& m = part_shape->MMaterial;
    if (!m->IsLoaded()) return NULL;

    return &GetExplosionParams(m->ExplosionType);
}

template <typename T>
class CScopedVariable
{
public:
    CScopedVariable() : Variable(), Original() {}
    CScopedVariable(T& variable, const T& value) : Variable(&variable), Original(variable) 
    {
        variable = value;
    }

    ~CScopedVariable()
    {
        if (Variable)
            *Variable = Original;
    }
public:
    void Set(T& variable, const T& value)
    {
        Variable = &variable;
        Original = variable;
        variable = value;
    }
private:
    T Original;
    T* Variable;
};


// Hacks for per-explosion decals
struct SNearExplodedThingInfo
{
    SNearExplodedThingInfo() : UID(), Params()
    {}

    SNearExplodedThingInfo(CThing* thing, const CExplosionParams* params) : 
    UID(thing != NULL ? thing->UID : 0), Params(params)
    {}

    SNearExplodedThingInfo(const SNearExplodedThingInfo& info) : UID(info.UID), Params(info.Params)
    {}

    SNearExplodedThingInfo& operator=(const SNearExplodedThingInfo& rhs)
    {
        UID = rhs.UID;
        Params = rhs.Params;
        return *this;
    }

    inline bool operator<(const SNearExplodedThingInfo& rhs) const
    {
        return UID < rhs.UID;
    }

    u32 UID;
    const CExplosionParams* Params;
};

CRawVector<SNearExplodedThingInfo> gNearExplodedThings(16);
extern StaticCP<RTexture> ScorchTex;
extern StaticCP<RFluidSettings> FluidSettings;
extern StaticCP<RMesh> ExplosionMesh;

MH_DefineFunc(SplatterScorchyStickers, 0x001c7038, TOC0, void, CThing*);
void zz_ProcessNearExplodedThingList(PWorld* world)
{
    if (gNearExplodedThings.size() == 0) return;
    std::sort(gNearExplodedThings.begin(), gNearExplodedThings.end(), std::less<SNearExplodedThingInfo>());
    DebugLog("processing %d nearby exploded things...\n", gNearExplodedThings.size());

    for (u32 i = 0; i < gNearExplodedThings.size(); ++i)
    {
        const SNearExplodedThingInfo& info = gNearExplodedThings[i];
        CThing* thing = world->GetThingByUID(info.UID);
        if (thing == NULL) continue;
        
        const CExplosionParams* params = info.Params;
        CScopedVariable<CP<RTexture> > _scoped_scorch_texture;
        CScopedVariable<CP<RFluidSettings> > _scoped_fluid_settings;
        if (params)
        {
            if (params->ScorchMarkTexture)
                _scoped_scorch_texture.Set(ScorchTex, params->ScorchMarkTexture);
            if (params->FluidSettings)
                _scoped_fluid_settings.Set(FluidSettings, params->FluidSettings);
        }

        SplatterScorchyStickers(thing);
    }

    gNearExplodedThings.resize(0);
}

MH_DefineFunc(PWorld_AddToNearExplodedThingList, 0x00034c38, TOC0, void, PWorld*, const CThing*, v2 const& a, v2 const& b);
void zz_AddToNearExplodedThingList(PWorld* world, const CThing* thing, v2 const& a, v2 const& b)
{
    CExplosionParams* params = GetExplosionParams(thing);
    if (!params) params = &GetExplosionParams(EXPLOSIVE_STYLE_STANDARD);

    if ((params->Flags & kExplosionFlags_DisableScorchMarks) != 0) return;

    PWorld_AddToNearExplodedThingList(world, thing, a, b);
    
    CVector<CThingPtr>& near_exploded_things = world->GetNearExplodedThings();
    for (u32 i = 0; i < near_exploded_things.size(); ++i)
        gNearExplodedThings.push_back(SNearExplodedThingInfo(near_exploded_things[i], params));
    
    near_exploded_things.resize(0);
}

void zz_AddExplosionBits(const CThing* thing, m44 pos)
{
    CScopedVariable<CP<RFluidSettings> > _scoped_fluid_settings;
    CScopedVariable<CP<RMesh> > _scoped_explosion_mesh;

    CP<RMaterial> material;

    CExplosionParams* params = GetExplosionParams(thing);
    if (params)
    {
        if (params->FluidSettings)
            _scoped_fluid_settings.Set(FluidSettings, params->FluidSettings);
        if (params->BitsMesh)
            _scoped_explosion_mesh.Set(ExplosionMesh, params->BitsMesh);
    }
    
    CFluidRender::AddExplosionBits(pos);
}

namespace NPoppetUtils
{
    MH_DefineFunc(ObjectSubtract, 0x003a3ef0, TOC1, void, const PYellowHead*, CThing*, CVector<CThingPtr>& objects, CCsgState&, bool);
    MH_DefineFunc(ObjectAdd, 0x003a33f8, TOC1, void, const PYellowHead*, CThing*, CVector<CThingPtr>& objects, CCsgState&);
}

void zz_PerformExplosiveCSG(const PYellowHead* player, CThing* n, CVector<CThingPtr>& objects, CCsgState& csg_state)
{
    CExplosionParams* params = GetExplosionParams(n);
    if (!params) params = &GetExplosionParams(EXPLOSIVE_STYLE_STANDARD);

    if (params->CsgType == EXPLOSIVE_CSG_CUT)
        NPoppetUtils::ObjectSubtract(player, n, objects, csg_state, true);
    else if (params->CsgType == EXPLOSIVE_CSG_ADD)
        NPoppetUtils::ObjectAdd(player, n, objects, csg_state);
}

void zz_OnPlayerHit(const CThing* player, const ExplosionInfo& info, v2 lethal_force)
{
    PCreature* part_creature = player->GetPCreature();
    const CExplosionParams& params = GetExplosionParams(info.ExplosionStyle);
    if ((params.Flags & kExplosionFlags_DontKillPlayer) == 0)
    {
        if (part_creature->GetState() == STATE_DEAD) return;
        part_creature->TypeOfLethalThingTouched = info.LethalType;
        part_creature->LethalForce = lethal_force;
        part_creature->SetState(STATE_DEAD);
        return;
    }

    switch (info.ExplosionStyle)
    {
        case EXPLOSIVE_STYLE_ICE:
        {
            part_creature->TypeOfLethalThingTouched = LETHAL_ICE;
            part_creature->SetState(STATE_FROZEN);
            break;
        }
        case EXPLOSIVE_STYLE_STUN:
        {
            part_creature->TypeOfLethalThingTouched = LETHAL_FIRE;
            part_creature->SetState(STATE_STUNNED);
            break;
        }
        case EXPLOSIVE_STYLE_LAUNCH:
        {
            break;
        }
        case EXPLOSIVE_STYLE_ADD:
        {
            break;
        }
    }
}

MH_DefineFunc(ApplyRadialForce, 0x0020f630, TOC0, void, ExplosionInfo const& info);

void GetExplosionInfo(CThing* thing, ExplosionInfo& info)
{
    PShape* shape;
    PPos* pos;
    new (&info) ExplosionInfo();
    if (thing == NULL || (shape = thing->GetPShape()) == NULL || (pos = thing->GetPPos()) == NULL) return;

    CP<RMaterial>& material = shape->MMaterial;
    if (!material->IsLoaded()) return;

    const CExplosionParams& params = GetExplosionParams(material->ExplosionType);

    info.Center = pos->Fork->WorldPosition * shape->COM.getCol3();
    info.InnerRadius = material->ExplosionMinRadius;
    info.OuterRadius = material->ExplosionMaxRadius;
    info.MinZ = shape->Fork->Min.getZ();
    info.MaxZ = shape->Fork->Max.getZ();

    info.MaxForce = material->ExplosionMaxForce;
    info.MaxVel = material->ExplosionMaxVel;
    info.MaxAngVel = material->ExplosionMaxAngVel;
    info.Flags = params.Flags;
    info.LethalType = params.LethalType;
    info.CsgType = params.CsgType;
    info.ExplosionStyle = params.Style;

    if (!params.Sound.empty())
        info.ExplosionSound = params.Sound.c_str();
}

bool InitializeExplosiveStyles()
{   
    GetExplosionParams(EXPLOSIVE_STYLE_SHOCK)
        .SetDisableParticles()
        .SetCSG(EXPLOSIVE_CSG_DISABLED)
        .SetLethalType(LETHAL_ELECTRIC)
        .SetSound("gameplay/lethal/electricity_explode");
    
    GetExplosionParams(EXPLOSIVE_STYLE_ICE)
        .SetDontKillPlayer()
        .SetFluidSettings(2365181022u)
        .SetScorchTexture(3728560712u)
        .SetBitsMesh(2925987700u)
        .SetCSG(EXPLOSIVE_CSG_CUT)
        .SetSound("gameplay/lethal/ice_break_free");
    
    GetExplosionParams(EXPLOSIVE_STYLE_STUN)
        .SetDontKillPlayer()
        .SetFluidSettings(2728552044u)
        .SetBitsMesh(4109350355u)
        .SetCSG(EXPLOSIVE_CSG_DISABLED)
        .SetSound("gameplay/lethal/fire_touch_stun");
    
    GetExplosionParams(EXPLOSIVE_STYLE_LAUNCH)
        .SetDontKillPlayer()
        .SetDisableParticles()
        .SetCSG(EXPLOSIVE_CSG_DISABLED)
        .SetSound("gameplay/lethal/electricity_eye_pop");
    
    GetExplosionParams(EXPLOSIVE_STYLE_ADD)
        .SetDisableParticles()
        .SetCSG(EXPLOSIVE_CSG_ADD)
        .SetSound("gameplay/lethal/death_explosion");

    return true;
}

extern "C" uintptr_t _radial_explosion_hook;
extern "C" uintptr_t _explosion_particle_and_sound_hook;
extern "C" uintptr_t _explosion_yellowhead_handler_hook;

void AttachExplosionHooks()
{
    for (int i = 0; i < NUM_EXPLOSIVE_STYLES; ++i)
        gExplosionParams[i].Style = (EExplosiveStyle)i;

    MH_PokeCall(0x00211700, zz_AddToNearExplodedThingList);
    MH_PokeHook(0x000309e0, zz_ProcessNearExplodedThingList);
    MH_InitHook((void*)0x00211510, (void*)&GetExplosionInfo);
    MH_PokeBranch(0x00211730, &_radial_explosion_hook);
    MH_PokeBranch(0x00211760, &_explosion_particle_and_sound_hook);
    MH_PokeBranch(0x0020f0f0, &_explosion_yellowhead_handler_hook);
    MH_PokeCall(0x00210f04, zz_PerformExplosiveCSG);
}