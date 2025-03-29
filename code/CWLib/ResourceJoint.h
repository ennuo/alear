#ifndef RESOURCE_JOINT_H
#define RESOURCE_JOINT_H

#include "Resource.h"

#include <MMString.h>
#include <refcount.h>

#include "ResourceGfxMaterial.h"
#include "ResourceGFXMesh.h"

#include "fcurve.h"

class RJoint : public CResource {
public:
    floatInV2 AngleElasticity;
    floatInV2 AnglePlasticity;
    floatInV2 AngleDeviation;
    floatInV2 AngleVelocity;
    fcurve AngleFunc;
    float LengthElasticity;
    float LengthPlasticity;
    fcurve LengthFunc;
    bool AllowExpand;
    bool AllowContract;
    bool NormalisedForces;
    bool DontRotateA;
    bool DontRotateB;
    bool ContractFreely;
    float BreakResistance;
    CP<RGfxMaterial> GfxMaterial;
    CP<RMesh> Mesh;
    MMString<char> EventNameLength;
    MMString<char> EventNameAngle;
};

#endif // RESOURCE_JOINT_H