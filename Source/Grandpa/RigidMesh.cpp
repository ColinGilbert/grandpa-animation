#include "Precompiled.h"
#include "RigidMesh.h"
#include "ContentResource.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
RigidMesh::RigidMesh(const RigidMeshResource* resource)
	: Mesh(resource)
	, m_resource(resource)
	, m_attachedBoneTransform(NULL)
{
	assert(resource != NULL);
	resource->grab();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
RigidMesh::~RigidMesh()
{
	assert(m_resource != NULL);
	m_resource->drop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const Resource* RigidMesh::getMeshResource() const
{
	return m_resource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RigidMesh::update()
{
	if (m_attachedBoneTransform != NULL)
	{
		assert(m_resource != NULL);
		m_transform = m_resource->getTransform() * (*m_attachedBoneTransform);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void RigidMesh::build()
{
	Mesh::build();

	assert(m_resource->getResourceState() == RES_STATE_COMPLETE);
	m_transform = m_resource->getTransform();

	m_staticStream.format = m_resource->getStaticStreamFormat();
	m_staticStream.stride = MeshFile::calculateVertexStride(m_staticStream.format);
	m_staticStream.buffer = const_cast<unsigned char*>(m_resource->getStaticVertexStream());

	setBuilt();
	calculateBoundingBox();
}

}
