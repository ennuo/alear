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

#include <algorithm>


class CNamedPolygon {
public:
    u32 guid;
    CRawVector<v2, CAllocatorMMAligned128> polygon;
};

CPoppetOutlineConfig gPoppetOutlineData;
extern CVector<CNamedPolygon> gNamedPolygons;

struct SortByPlanKey
{
    bool operator()(CPoppetOutline& a, CPoppetOutline& z) const
    {
        return a.Plan < z.Plan;
    }
};

bool LoadPoppetMeshOutlines()
{
    gPoppetOutlineData.Outlines.clear();
    

    DebugLog("attempting to load poppet mesh outlines...");
    CP<RFileOfBytes> file = LoadResourceByKey<RFileOfBytes>(E_OUTLINES_KEY, 0, STREAM_PRIORITY_DEFAULT);
    
    DebugLog("%s\n", file.GetRef());

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

    std::sort(gPoppetOutlineData.Outlines.begin(), gPoppetOutlineData.Outlines.end(), SortByPlanKey());

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
    
    for (CPoppetOutline* poppet_outline = gPoppetOutlineData.Outlines.begin(); poppet_outline != gPoppetOutlineData.Outlines.end(); ++poppet_outline)
    {
        DebugLog("DEADBEEF! Attempting to load poppet outline object g%08x\n", poppet_outline->Plan);
        
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
        {
            DebugLog("found outline guid for g%d -> g%d\n", mesh_guid, outline->Plan);
            return outline->Plan;
        }
    }

    DebugLog("Couldn't find outline GUID for g%d\n", mesh_guid);

    return 0;
}