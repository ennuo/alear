#include "RenderJoint.h"
#include "AlearSerialization.h"
#include "AlearConfig.h"

#include <vector.h>

#include <ResourceSystem.h>
#include <ResourceFileOfBytes.h>
#include <Variable.h>

#include <cell/DebugLog.h>

CRenderJoint gRenderJoints[JOINT_TYPE_MAX];
const char* gJointNames[JOINT_TYPE_MAX + 1] = {
    "JOINT_TYPE_LEGACY",
    "JOINT_TYPE_ELASTIC",
    "JOINT_TYPE_SPRING",
    "JOINT_TYPE_CHAIN",
    "JOINT_TYPE_PISTON",
    "JOINT_TYPE_STRING",
    "JOINT_TYPE_ROD",
    "JOINT_TYPE_BOLT",
    "JOINT_TYPE_SPRING_ANGULAR",
    "JOINT_TYPE_MOTOR",
    "JOINT_TYPE_QUANTIZED",
    "JOINT_TYPE_MAX"
};

void CRenderJoint::LoadMeshResources()
{
    if (Mesh != 0) MeshResource = LoadResourceByKey<RMesh>(Mesh, 0, STREAM_PRIORITY_DEFAULT);
    if (InactiveMesh != 0) InactiveMeshResource = LoadResourceByKey<RMesh>(InactiveMesh, 0, STREAM_PRIORITY_DEFAULT);
    if (PatternMesh != 0) PatternMeshResource = LoadResourceByKey<RMesh>(PatternMesh, 0, STREAM_PRIORITY_DEFAULT);
}

bool LoadJointMeshes()
{
    tGatherElementMap lookup;
    for (int i = 0; i <= JOINT_TYPE_MAX; ++i)
        lookup.insert(tGatherElementMap::value_type(gJointNames[i], i));

    CP<RFileOfBytes> file = LoadResourceByKey<RFileOfBytes>(E_JOINTS_KEY, 0, STREAM_PRIORITY_DEFAULT);

    file->BlockUntilLoaded();
    if (!file->IsLoaded()) return false;

    ByteArray& b = file->GetData();
    CGatherVariables variables;

    CRenderJoints joints;
    variables.Init<CRenderJoints>(&joints);

    CScopedGatherLookup _gather_scope(&lookup);

    if (GatherVariablesLoad(b, variables, true, NULL) != REFLECT_OK)
    {
        DebugLog("An error occurred while loading data for joint meshes!\n");
        return false;
    }
    
    // Copy all the data from the vector into our global array
    for (int i = 0; i < joints.Joints.size(); ++i)
    {   
        gRenderJoints[i] = joints.Joints[i];

        CRenderJoint& joint = gRenderJoints[i];
        if (!gUseAlternateJointMeshes || joint.InactiveMesh == 0) { joint.InactiveMesh = joint.Mesh; }
        if (!gUseAlternateJointMeshes || joint.PatternMesh == 0) { joint.PatternMesh = joint.Mesh; }
        
        joint.LoadMeshResources();
    }

    return true;
}

CP<RMesh> GetRenderJointMesh(PJoint* joint)
{
    if (joint->Type == JOINT_TYPE_MOTOR && joint->AnimationPattern != 1) LoadResourceByKey<RMesh>(21704, 0, STREAM_PRIORITY_DEFAULT);
    if (joint->Type == JOINT_TYPE_LEGACY)
    {
        if (!joint->Settings || !joint->Settings->IsLoaded()) return NULL;
        return joint->Settings->Mesh;
    }

    if (joint->Type < JOINT_TYPE_LEGACY || joint->Type >= JOINT_TYPE_MAX) return NULL;

    CRenderJoint& render_joint = gRenderJoints[joint->Type];
    if (joint->GetB() == NULL) return render_joint.GetInactiveMeshResource();
    return render_joint.GetMeshResource();
}