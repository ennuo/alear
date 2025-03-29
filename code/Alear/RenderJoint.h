#ifndef RENDER_JOINT_H
#define RENDER_JOINT_H

#include <refcount.h>
#include <vector.h>

#include <PartPhysicsJoint.h> 
#include <ResourceGFXMesh.h>

class RMesh;

class CRenderJoint {
public:
    CRenderJoint() : Mesh(0), InactiveMesh(0), MeshResource(NULL), InactiveMeshResource(NULL)
    {}

    CRenderJoint(const CRenderJoint& joint) : Mesh(0), InactiveMesh(0), MeshResource(NULL), InactiveMeshResource(NULL)
    {
        *this = joint;
    }

    CRenderJoint& operator=(const CRenderJoint& rhs)
    {
        Mesh = rhs.Mesh;
        InactiveMesh = rhs.InactiveMesh;
        MeshResource = rhs.MeshResource;
        InactiveMeshResource = rhs.InactiveMeshResource;

        return *this;
    }
public:
    inline const CP<RMesh>& GetMeshResource() const { return MeshResource; }
    inline const CP<RMesh>& GetInactiveMeshResource() const { return InactiveMeshResource; }
public:
    void LoadMeshResources();
public:
    u32 Mesh;
    u32 InactiveMesh;
private:
    CP<RMesh> MeshResource;
    CP<RMesh> InactiveMeshResource;
};

class CRenderJoints {
public:
    CRenderJoints() : Joints() {}
public:
    CVector<CRenderJoint> Joints;
};

extern CRenderJoint gRenderJoints[JOINT_TYPE_MAX];
extern const char* gJointNames[JOINT_TYPE_MAX + 1];

enum { E_JOINTS_KEY = 2884316639u };

bool LoadJointMeshes();
CP<RMesh> GetRenderJointMesh(PJoint* joint);

#endif // RENDER_JOINT_H