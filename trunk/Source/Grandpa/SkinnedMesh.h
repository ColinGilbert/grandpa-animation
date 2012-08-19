#ifndef __GRP_SKINNED_MESH_H__
#define __GRP_SKINNED_MESH_H__

#include "Mesh.h"
#include "ISkin.h"
#include "IResource.h"
#include <vector>

namespace grp
{

////////////////////////////////////////////////////////////////////////////////////////////////////
//to prevent ambiguous... there's another user data in IMesh
class Skin : public ISkin
{
public:
	Skin() : m_userData(NULL)
	{
	}
	virtual void setUserData(void* data)
	{
		m_userData = data;
	}
	virtual void* getUserData() const
	{
		return m_userData;
	}
private:
	void* m_userData;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class SkinnedMeshFile;
template<class T, ResourceType resType> class ContentResource;
typedef ContentResource<SkinnedMeshFile, RES_TYPE_SKINNED_MESH> SkinnedMeshResource;

class SkinnedMesh : public Mesh, public Skin
{
public:
	SkinnedMesh(const SkinnedMeshResource* resource);
	virtual ~SkinnedMesh();

	virtual const Resource* getMeshResource() const;

	virtual const Matrix& getTransform() const;

	virtual void build();

	virtual ISkin* getSkin();

	virtual void setUpdateMode(MeshUpdateMode mode);
	virtual MeshUpdateMode getUpdateMode() const;

	//from ISkin
	virtual size_t getBoneCount() const;
	virtual const Matrix* getBoneMatrices() const;
	virtual size_t getSkinVertexCount() const;
	virtual const VertexInfluence* getVertexInfluences(size_t vertexIndex) const;
	virtual void setGpuSkinning(bool enable);
	virtual bool isGpuSkinning() const;
	
	virtual size_t getBBVertexCount() const;

public:
	void setBoneMatrix(unsigned long boneIndex, int boneId, const Matrix* matrix);

	const VECTOR(int)& getBoneIds() const;

	void update();

	void enableWeightLod(bool enable = true);

private:
	void updateVertex();
	void updateVertex_NoTangent();
	void updateVertex_PosOnly();

private:
	const SkinnedMeshResource*	m_resource;
	
	VECTOR(int)				m_boneIds;
	VECTOR(const Matrix*)	m_boneTransforms;
	VECTOR(Matrix)			m_finalBoneTransforms;

	MeshUpdateMode			m_updateMode;
	bool					m_gpuSkinning;
	bool					m_weightLod;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void SkinnedMesh::setBoneMatrix(unsigned long boneIndex, int boneId, const Matrix* matrix)
{
	assert(boneIndex < m_boneTransforms.size());
	assert(boneIndex < m_boneIds.size());
	m_boneTransforms[boneIndex] = matrix;
	m_boneIds[boneIndex] = boneId;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void SkinnedMesh::enableWeightLod(bool enable)
{
	m_weightLod = enable;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Matrix& SkinnedMesh::getTransform() const
{
	return Matrix::IDENTITY;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void SkinnedMesh::setUpdateMode(MeshUpdateMode mode)
{
	m_updateMode = mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline MeshUpdateMode SkinnedMesh::getUpdateMode() const
{
	return m_updateMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ISkin* SkinnedMesh::getSkin()
{
	return this;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t SkinnedMesh::getBoneCount() const
{
	return m_finalBoneTransforms.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Matrix* SkinnedMesh::getBoneMatrices() const
{
	return &m_finalBoneTransforms[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool SkinnedMesh::isGpuSkinning() const
{
	return m_gpuSkinning;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(int)& SkinnedMesh::getBoneIds() const
{
	return m_boneIds;
}

}

#endif
