#ifndef __STANDALONE_RIGID_MESH_H__
#define __STANDALONE_RIGID_MESH_H__

#include "RigidMesh.h"

namespace grp
{

class StandaloneRigidMesh : public RigidMesh
{
public:
	StandaloneRigidMesh(const RigidMeshResource* resource);

	virtual void build();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline StandaloneRigidMesh::StandaloneRigidMesh(const RigidMeshResource* resource)
	: RigidMesh(resource)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void StandaloneRigidMesh::build()
{
	RigidMesh::build();

	m_type |= MESH_STANDALONE;
}

}

#endif
