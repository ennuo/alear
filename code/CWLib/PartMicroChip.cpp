#include <PartMicroChip.h>
#include <PartPhysicsWorld.h>
#include <thing.h>
#include <ResourceSystem.h>
#include <ResourceMaterial.h>
#include <ResourceGfxMaterial.h>

enum
{
    E_KEY_BASE_CIRCUIT_BOARD_MAT = 75328,
    E_KEY_PROC_PCB_GMAT = 76711
};

bool PMicroChip::IsCircuitBoardVisible() const
{
    return CircuitBoardThing && CircuitBoardThing->HasPart(PART_TYPE_POS);
}

void PMicroChip::ShowCircuitBoard(bool visible)
{
    PPos* part_pos = GetThing()->GetPPos();
    if (!part_pos) return;

    v2 circuitboard_size = v2(CircuitBoardSizeX, CircuitBoardSizeY);
    if (visible)
    {
        CircuitBoardThing->AddPart(PART_TYPE_POS);
        CircuitBoardThing->AddPart(PART_TYPE_BODY);
        CircuitBoardThing->AddPart(PART_TYPE_SHAPE);
        CircuitBoardThing->AddPart(PART_TYPE_GENERATED_MESH);

        PShape* part_shape = CircuitBoardThing->GetPShape();
        PGeneratedMesh* part_mesh = CircuitBoardThing->GetPGeneratedMesh();

        CRawVector<v2, CAllocatorMMAligned128> polygon;
        if (IsSequencer())
        {
            polygon.push_back(v2(1.0f, 0.0f) * circuitboard_size);
            polygon.push_back(v2(1.0f, -1.0f) * circuitboard_size);
            polygon.push_back(v2(0.0f, -1.0f) * circuitboard_size);
            polygon.push_back(v2(0.0f, 0.0f) * circuitboard_size);
        }
        else
        {
            polygon.push_back(v2(1.0f, 1.0f) * circuitboard_size);
            polygon.push_back(v2(1.0f, -1.0f) * circuitboard_size);
            polygon.push_back(v2(-1.0f, -1.0f) * circuitboard_size);
            polygon.push_back(v2(-1.0f, 1.0f) * circuitboard_size);
        }

        CRawVector<unsigned int> indices;
        indices.push_back(4);

        CP<RMaterial> phys = LoadResourceByKey<RMaterial>(E_KEY_BASE_CIRCUIT_BOARD_MAT);
        phys->BlockUntilLoaded();

        part_shape->SetMaterial(phys);
        part_shape->SetPolygon(polygon, indices);
        part_shape->SetThickness(10.0f);
        part_shape->UpdateMassDepth();
        part_shape->COM = m44::identity();

        part_mesh->GfxMaterial = LoadResourceByKey<RGfxMaterial>(E_KEY_PROC_PCB_GMAT);
        part_mesh->GfxMaterial->BlockUntilLoaded();
    }
    else
    {
        CircuitBoardThing->SetParent(NULL);
        CircuitBoardThing->RemovePart(PART_TYPE_GENERATED_MESH);
        CircuitBoardThing->RemovePart(PART_TYPE_BODY);
        CircuitBoardThing->RemovePart(PART_TYPE_SHAPE);
        CircuitBoardThing->RemovePart(PART_TYPE_POS);
        CircuitBoardThing->RemovePart(PART_TYPE_DECORATIONS);
        CircuitBoardThing->SetParent(GetThing());

        AnimationTarget = 0.0f;
        AnimationCurrent = 0.0f;
        AnimationDelta = 0.0f;
    }

}

PMicroChip::~PMicroChip()
{
    if (CircuitBoardThing)
        delete CircuitBoardThing.GetThing();
}

void PMicroChip::UpdateRenderDepths()
{
    
}

void PMicroChip::CreateCircuitBoard()
{
    // Normally in LBP2, this function has a lot of additional logic for
    // handling legacy circuitboards being converted, and it also loads
    // the circuitboard itself from a plan file, but there's really no reason
    // to do that when a closed circuit board is just an empty thing with a PSwitch.

    CircuitBoardThing = new CThing();
    CircuitBoardThing->SetWorld(GetThing()->World, 0);
    CircuitBoardThing->SetParent(GetThing());
    // CircuitBoardThing->AddPart(PART_TYPE_SWITCH);
    // CircuitBoardThing->GetPSwitch()->Type = SWITCH_TYPE_CIRCUIT_BOARD;

    CircuitBoardThing->UpdateObjectType();
    GetThing()->UpdateObjectType();
    ShowCircuitBoard(false);
}

#include <MatrixUtils.h>
void PMicroChip::UpdateRenderPos()
{
    if (!GetThing()->HasPart(PART_TYPE_POS) || !IsCircuitBoardVisible()) return;

    const PPos* part_pos = GetThing()->GetPPos();

    // v4 translation; q4 rotation; v3 scale;
    // Decompose(translation, rotation, scale, part_pos->Fork->WorldPosition);

    

    v4 offset = part_pos->Fork->WorldPosition.getCol3() + (Offset * AnimationCurrent);
    offset.setW(1.0f);




    CircuitBoardThing->GetPPos()->SetWorldPos(m44(
        v4::xAxis(), // * MAX(abs(AnimationCurrent), 0.1f),
        v4::yAxis(), // * MAX(abs(AnimationCurrent), 0.1f),
        v4::zAxis(),
        offset
    ), false, 0);
}

void PMicroChip::UpdateAnimation()
{
    if (!IsCircuitBoardVisible())
    {
        if (AnimationTarget != 1.0f) return;
        ShowCircuitBoard(true);
    }

    if (AnimationCurrent == 0.0f && AnimationTarget == 0.0f)
    {
        AnimationCurrent = 1.0f;
        AnimationTarget = 1.0f;
    }

    AnimationCurrent += AnimationDelta;

    if (AnimationTarget == 1.0f)
        AnimationDelta = AnimationDelta * 0.5f + ((AnimationTarget - AnimationCurrent) * 0.5f);
    else
        AnimationDelta = -0.3f;

    if (AnimationTarget == 0.0f && AnimationCurrent <= 0.0f)
    {
        ShowCircuitBoard(false);
        AnimationCurrent = 0.0f;
        AnimationDelta = 0.0f;
    }
}

void PMicroChip::SetCircuitBoardVisible(bool visible)
{
    if (visible)
    {
        if (!CircuitBoardThing) CreateCircuitBoard();
        if (GetThing()->World != NULL)
            LastTouched = GetThing()->World->GetFrame();
        ShowCircuitBoard(true);
        AnimationTarget = 1.0f;
        CAudio::PlaySample(CAudio::gSFX, "poppet/circuitboard_open", CircuitBoardThing);
    }
    else
    {
        AnimationTarget = 0.0f;
        if (CircuitBoardThing)
        {
            if (CircuitBoardThing->HasPart(PART_TYPE_SHAPE))
            {
                v2 pos = CircuitBoardThing->GetPShape()->GetPosCOM();
                CAudio::PlaySample(CAudio::gSFX, "poppet/circuitboard_close", CAudio::DEFAULT_PARAM, &pos);
            }
        }
    }


}
