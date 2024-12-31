#include <PartSwitch.h>
#include <Poppet.h>
#include <thing.h>
#include <ResourceGame.h>
#include <PartPhysicsWorld.h>
#include <hook.h>
#include <Variable.h>
#include <cell/DebugLog.h>

namespace NPoppetUtils
{
    MH_DefineFunc(RaySphereIntersect, 0x0038eedc, TOC1, bool, v4 center, float radius, v4 ray_pos, v4 ray_dir, float& intersect_dist);
}

// MH_DefineFunc(GetOutlinePolygon, 0x0035bbc4, )

MH_DefineFunc(PSwitch_RaycastConnector, 0x0004d764, TOC0, bool, PSwitch*, v4, v4, float&, CThing*&);
bool PSwitch::RaycastConnector(v4 start, v4 dir, float& t, CThing*& hit)
{
    return PSwitch_RaycastConnector(this, start, dir, t, hit);
}

void CPoppet::InitializeExtraData()
{
    DebugLog("\n\n\n\n\n\n\nALEAR MARKER !!! POOPY FARTY !!! INITING HIDDEN LIST\n\n\n\n");
    
    new (&HiddenList) CVector<CThingPtr>();
}

void CPoppet::DestroyExtraData()
{
    HiddenList.~CVector();
}

void CustomRaycastAgainstSwitches(CPoppet* poppet)
{
    poppet->m_bestTFromPSwitches = 1.0e+20f;
    poppet->m_havePSwitchHit = false;

    if (poppet->GetSubMode() != SUBMODE_NONE) return;

    PWorld* world = gGame->GetWorld();
    CThing* ignored = poppet->GetThingToIgnore();

    CRaycastResults& raycast = poppet->m_raycastResultFromPSwitches;
    raycast.HitPort = -1;
    raycast.RefPort = -1;

    float t;
    CThing* hit;

    for (PSwitch** it = world->ListPSwitch.begin(); it != world->ListPSwitch.end(); ++it)
    {
        PSwitch* sw = (*it);
        CThing* thing = sw->GetThing();

        if (thing == ignored) continue;

        if (sw->Type == 14)
        {
            m44& wpos = thing->GetPPos()->Game.WorldPosition;

            v4 input_offsets[] =
            {
                wpos * v4(-60.0f, 35.0f, 0.0f, 1.0f),
                wpos * v4(-60.0f, -35.0f, 0.0f, 1.0f)
            };

            v4 output_offsets[] =
            {
                wpos * v4(100.0f, 0.0f, 0.0f, 1.0f)
            };

            for (int i = 0; i < 1; ++i)
            {
                v4& port_offset = output_offsets[i];
                if (!NPoppetUtils::RaySphereIntersect(port_offset, 20.0f, poppet->m_rayStart, poppet->m_rayDir, t)) continue;
                if (t >= 1.0e+20f) continue;

                DebugLog("hitting output port %d on AND gate!!!\n", i);

                raycast.SwitchConnector = true;
                raycast.HitThing = thing;
                raycast.RefThing = thing;

                poppet->m_bestTFromPSwitches = t;
                poppet->m_havePSwitchHit = true;

                raycast.BaryU = 0.0f;
                raycast.BaryV = 0.0f;
                raycast.Normal = v4(0.0f, 0.0f, 1.0f, 0.0f);
                raycast.TriIndex = 0;
                raycast.OnCostumePiece = -1;
                raycast.DecorationIdx = -1;

                raycast.HitPort = i;
            }

            for (int i = 0; i < 2; ++i)
            {
                v4& port_offset = input_offsets[i];
                if (!NPoppetUtils::RaySphereIntersect(port_offset, 20.0f, poppet->m_rayStart, poppet->m_rayDir, t)) continue;
                if (t >= 1.0e+20f) continue;

                DebugLog("hitting input port %d on AND gate!!!\n", i);

                raycast.SwitchConnector = false;
                raycast.HitThing = thing;
                raycast.RefThing = thing;
                
                poppet->m_bestTFromPSwitches = t;
                poppet->m_havePSwitchHit = true;

                raycast.BaryU = 0.0f;
                raycast.BaryV = 0.0f;
                raycast.Normal = v4(0.0f, 0.0f, 1.0f, 0.0f);
                raycast.TriIndex = 0;
                raycast.OnCostumePiece = -1;
                raycast.DecorationIdx = -1;

                raycast.RefPort = i;
            }

            // input ports are a sphere with a radius of 20
                // at x = -60
                // y = -35 / 35

            continue;
        }

        if (!sw->RaycastConnector(poppet->m_rayStart, poppet->m_rayDir, t, hit)) continue;
        if (t >= 1.0e+20f) continue;

        raycast.SwitchConnector = true;

        raycast.HitThing = hit;
        raycast.RefThing = thing;

        poppet->m_bestTFromPSwitches = t;
        poppet->m_havePSwitchHit = true;

        raycast.BaryU = 0.0f;
        raycast.BaryV = 0.0f;
        raycast.Normal = v4(0.0f, 0.0f, 1.0f, 0.0f);
        raycast.TriIndex = 0;
        raycast.OnCostumePiece = -1;
        raycast.DecorationIdx = -1;

        raycast.HitPort = 0;
        raycast.RefPort = 0;
    }
}

#define ADD(name) ret = Add(r, d.name, #name); if (ret != REFLECT_OK) return ret;
template<typename R>
ReflectReturn Reflect(R& r, CRaycastResults& d)
{
    ReflectReturn ret;
    ADD(HitPoint);
    ADD(Normal);
    ADD(BaryU);
    ADD(BaryV);
    ADD(TriIndex);
    ADD(HitThing);
    ADD(RefThing);
    ADD(OnCostumePiece);
    ADD(DecorationIdx);
    ADD(SwitchConnector);
    ADD(HitPort);
    ADD(RefPort);
    return ret;
}

template ReflectReturn Reflect<CGatherVariables>(CGatherVariables& r, CRaycastResults& d);

#undef ADD

ReflectReturn GatherPoppetRaycastVariables(CGatherVariables& r, CRaycastResults& d)
{
    return Reflect(r, d);
}

extern "C" uintptr_t _raycast_hook;
extern "C" uintptr_t _popit_destroy_extra_data_hook;
extern "C" uintptr_t _popit_init_extra_data_hook;

void InitLogicSystemHooks()
{    
    // MH_InitHook((void*)0x00353370, (void*)&CustomRaycastAgainstSwitches);
    // MH_InitHook((void*)0x007716e8, (void*)&GatherPoppetRaycastVariables);

    // replace stb with stw for collating raycast so it copies additional fields
    MH_Poke32(0x00356aa0, 0x815f004c /* lwz %r10, 0x4c(%r31) */);
    MH_Poke32(0x00356acc, 0x915e18fc /* stw %r10, 0x18fc(%r30) */);

    // raycast hack for local players
    MH_PokeBranch(0x0011067c, &_raycast_hook);

    // Increase the allocation size for the poppet class instance
    MH_Poke32(0x0073b5dc, LI(4, sizeof(CPoppet)));
    MH_Poke32(0x00231bdc, LI(4, sizeof(CPoppet)));
    MH_Poke32(0x000316bc, LI(4, sizeof(CPoppet)));

    // Add hooks for our custom data construction/destruction
    MH_PokeBranch(0x0034f264, &_popit_init_extra_data_hook);
    MH_PokeBranch(0x0034b4f4, &_popit_destroy_extra_data_hook);
}