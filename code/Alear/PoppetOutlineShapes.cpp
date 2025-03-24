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

class CNamedPolygon {
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

    return true;
}

void LoadOutlinePolygons()
{
    CMainGameStageOverride _stage_override(E_UPDATE_STAGE_OTHER_WORLD);

    if (gPoppetOutlineData.Outlines.empty())
        LoadPoppetMeshOutlines();

    gNamedPolygons.try_resize(0);
    gNamedPolygons.try_reserve(gPoppetOutlineData.Outlines.size());

    CThing* world_thing = new CThing();
    world_thing->AddPart(PART_TYPE_WORLD);
    
    for (CPoppetOutline* poppet_outline = gPoppetOutlineData.Outlines.begin(); poppet_outline != gPoppetOutlineData.Outlines.end(); ++poppet_outline)
    {
        DebugLog("DEADBEEF! Attempting to load poppet outline object g%08x\n", poppet_outline->Plan.guid);
        
        CP<RPlan> plan = LoadResourceByKey<RPlan>(poppet_outline->Plan.guid, 0, STREAM_PRIORITY_DEFAULT);
        plan->BlockUntilLoaded();

        if (!plan->IsLoaded()) continue;

        NetworkPlayerID player_id(INVALID_PLAYER_ID);
        CThing* thing = RPlan::MakeClone(plan, world_thing->GetPWorld(), player_id, true);
        if (thing == NULL || thing->GetPShape() == NULL || thing->GetPPos() == NULL) continue;

        PShape* part_shape = thing->GetPShape();
        PPos* part_pos = thing->GetPPos();

        CNamedPolygon polygon;
        polygon.guid = poppet_outline->Plan.guid;

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

CGUID GetOutlinePlanGUID(CGUID mesh_guid)
{
    CVector<CPoppetOutline>& outlines = gPoppetOutlineData.Outlines;
    for (CPoppetOutline* outline = outlines.begin(); outline != outlines.end(); ++outline)
    {
        if (outline->Mesh == mesh_guid)
            return outline->Plan;
    }

    return 0;
}