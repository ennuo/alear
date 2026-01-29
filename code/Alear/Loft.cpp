#include <SceneGraph.h>
#include <ResourceSettingsSoftPhys.h>
#include <ResourceGame.h>

#include <thing.h>
#include <PartPhysicsWorld.h>
#include <PartEffector.h>

#include <cell/DebugLog.h>


#define SOFTBODY_CACHED_WIND (48)
#define SOFTBODY_CACHED_WIND_STRENGTH (52)
#define SOFTBODY_CACHED_ENABLED (53)
#define SOFTBODY_CACHE_MAGIC (0x43535048)

#define SOFTBODY_GET_WIND(softphys) *(v2*)(softphys->ClusterEffect + SOFTBODY_CACHED_WIND)
#define SOFTBODY_GET_WIND_STRENGTH(softphys) softphys->ClusterEffect[SOFTBODY_CACHED_WIND_STRENGTH]
#define SOFTBODY_HAS_CACHED_DATA(softphys) (*(int*)(softphys->ClusterEffect + SOFTBODY_CACHED_ENABLED) == SOFTBODY_CACHE_MAGIC)

#define SOFTBODY_CACHE_DATA(softphys) \
    *(v2*)(softphys->ClusterEffect + SOFTBODY_CACHED_WIND) = softphys->Wind; \
    softphys->ClusterEffect[SOFTBODY_CACHED_WIND_STRENGTH] = softphys->WindStrength; \
    *(int*)(softphys->ClusterEffect + SOFTBODY_CACHED_ENABLED) = SOFTBODY_CACHE_MAGIC;

void OnSoftbodySim(CMeshInstance* instance, RSettingsSoftPhys* softphys)
{
    CThing* thing;
    CThing* world;
    PWorld* pworld;
    PEffector* effector;

    if ((thing = instance->MyThing) == NULL) return;
    if ((pworld = thing->World) == NULL) return;
    if ((world = pworld->GetThing()) == NULL) return;
    if ((effector = world->GetPEffector()) == NULL) return;

    if (!SOFTBODY_HAS_CACHED_DATA(softphys))
    {
        SOFTBODY_CACHE_DATA(softphys);
    }

    softphys->Wind = SOFTBODY_GET_WIND(softphys) + effector->PosVel;
    softphys->WindStrength = SOFTBODY_GET_WIND_STRENGTH(softphys) + (effector->WindStrength / 50.0f);
}
