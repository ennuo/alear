#include "PoppetOutlineShapes.h"

#include <vector.h>
#include <refcount.h>

#include <Variable.h>
#include <ResourceFileOfBytes.h>
#include <ResourceSystem.h>
#include <GuidHash.h>
#include <GameUpdateStage.h>
#include <ResourcePlan.h>
#include <PartPhysicsWorld.h>

#include <network/NetworkUtilsNP.h>
#include <mem_stl_buckets.h>

#include <algorithm>
#include <set>


class CNamedPolygon {
public:
    inline bool operator<(const CNamedPolygon& rhs) const
    {
        return guid < rhs.guid;
    }
public:
    u32 guid;
    CRawVector<v2, CAllocatorMMAligned128> polygon;
};

CPoppetOutlineConfig gPoppetOutlineData;
extern CVector<CNamedPolygon> gNamedPolygons;

bool LoadPoppetMeshOutlines()
{
    gPoppetOutlineData.Outlines.clear();
    
    CP<RFileOfBytes> file = LoadResourceByKey<RFileOfBytes>(E_OUTLINES_KEY, 0, STREAM_PRIORITY_DEFAULT);

    file->BlockUntilLoaded();
    if (!file->IsLoaded()) return false;

    ByteArray& b = file->GetData();
    CGatherVariables variables;
    variables.Init<CPoppetOutlineConfig>(&gPoppetOutlineData);
    if (GatherVariablesLoad(b, variables, true, NULL) != REFLECT_OK)
    {
        DebugLog("An error occurred while loading data for outline shapes!\n");
        return false;
    }

    // The game uses a binary search to find outlines, so make sure we're sorted in ascending order.
    std::sort(gPoppetOutlineData.Outlines.begin(), gPoppetOutlineData.Outlines.end(), std::less<CPoppetOutline>());

    DebugLog("Loaded %d outlines...\n", gPoppetOutlineData.Outlines.size());
    for (int i = 0; i < gPoppetOutlineData.Outlines.size(); ++i)
    {
        const CPoppetOutline& outline = gPoppetOutlineData.Outlines[i];
        DebugLog("[%d] [Mesh]=g%08x [Plan]=g%08x\n", i, outline.Mesh, outline.Plan);
    }

    return true;
}

bool gHasLoadedPolygons;

void LoadOutlinePolygons()
{
    CMainGameStageOverride _stage_override(E_UPDATE_STAGE_OTHER_WORLD);

    gNamedPolygons.try_resize(0);
    gNamedPolygons.try_reserve(gPoppetOutlineData.Outlines.size());

    CThing* world_thing = new CThing();
    world_thing->AddPart(PART_TYPE_WORLD);

    std::set<u32, std::less<u32>, STLBucketAlloc<u32> > plans_processed;

    DebugLog("Caching named polygons...\n");

    for (CPoppetOutline* poppet_outline = gPoppetOutlineData.Outlines.begin(); poppet_outline != gPoppetOutlineData.Outlines.end(); ++poppet_outline)
    {
        if (plans_processed.find(poppet_outline->Plan) != plans_processed.end())
        {
            DebugLog("Skipping load of outline for mesh g%08x since we already have plan g%08x cached!\n", poppet_outline->Mesh, poppet_outline->Plan);
            continue;
        }

        plans_processed.insert(poppet_outline->Plan);

        DebugLog("Caching poppet outline object g%08x\n", poppet_outline->Plan);
        
        CP<RPlan> plan = LoadResourceByKey<RPlan>(poppet_outline->Plan, 0, STREAM_PRIORITY_DEFAULT);
        plan->BlockUntilLoaded();

        if (!plan->IsLoaded()) continue;

        NetworkPlayerID player_id(INVALID_PLAYER_ID);
        CThing* thing = RPlan::MakeClone(plan, world_thing->GetPWorld(), player_id, true);
        if (thing == NULL || thing->GetPShape() == NULL || thing->GetPPos() == NULL) continue;

        PShape* part_shape = thing->GetPShape();
        PPos* part_pos = thing->GetPPos();

        CNamedPolygon polygon;
        polygon.guid = poppet_outline->Plan;

        CRawVector<v2, CAllocatorMMAligned128>& vertices = polygon.polygon;
        vertices = thing->GetPShape()->Polygon;

        v2 inv = part_pos->Fork->WorldPosition * part_shape->COM.getCol3();
        for (v2* it = vertices.begin(); it != vertices.end(); ++it)
        {
            v2& v = *it;
            v.setZ(0.0f);
            v = v - inv;
        }

        gNamedPolygons.push_back(polygon);
    }

    DebugLog("Finished loading named polygons! Count: %d\n", gNamedPolygons.size());
    for (int i = 0; i < gNamedPolygons.size(); ++i)
    {
        const CNamedPolygon& polygon = gNamedPolygons[i];
        DebugLog("[%d] [PlanGUID]=g%08x, [NumVerts] = %d\n", i, polygon.guid, polygon.polygon.size());
    }

    delete world_thing;
}

u32 GetOutlinePlanGUID(u32 mesh_guid)
{
    if (!gHasLoadedPolygons)
    {
        LoadPoppetMeshOutlines();
        LoadOutlinePolygons();
        gHasLoadedPolygons = true;
    }

    CVector<CPoppetOutline>& outlines = gPoppetOutlineData.Outlines;
    for (CPoppetOutline* outline = outlines.begin(); outline != outlines.end(); ++outline)
    {
        if (outline->Mesh == mesh_guid)
            return outline->Plan;
    }

    return 0;
}