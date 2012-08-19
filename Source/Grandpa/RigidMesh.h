#ifndef __GRP_RIGID_MESH_H__
#define __GRP_RIGID_MESH_H__

#include "Mesh.h"
#include "IResource.h"

namespace grp
{

class RigidMeshFile;
template<class T, ResourceType resType> class ContentResource;
typedef ContentResource<RigidMeshFile, RES_TYPE_RIGID_MESH> RigidMeshResource;

///////////////////////////////////////////////////////////////////////////////////////////////////
class RigidMesh : public Mesh
{
public:
	RigidMesh(const RigidMeshResource* resource);
	virtual ~RigidMesh();

	virtual const Resource* getMeshResource() const;

	virtual const Matrix& getTransform() const;

	void update();

	virtual void build();

	void setAttachedBoneMatrix(const Matrix& transform);
	const Matrix* getAttachedBoneMatrix() const;

private:
	const RigidMeshResource*	m_resource;
	const Matrix*				m_attachedBoneTransform;
	Matrix						m_transform;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void RigidMesh::setAttachedBoneMatrix(const Matrix& transform)
{
	m_attachedBoneTransform = &transform;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Matrix* RigidMesh::getAttachedBoneMatrix() const
{
	return m_attachedBoneTransform;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Matrix& RigidMesh::getTransform() const
{
	return m_transform;
}

}

#endif
