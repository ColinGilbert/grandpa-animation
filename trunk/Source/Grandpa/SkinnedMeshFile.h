#ifndef __GRP_SKINNED_MESH_FILE_H__
#define __GRP_SKINNED_MESH_FILE_H__

#include "MeshFile.h"
#include "ISkin.h"
#include <vector>

namespace grp
{

const float MIN_VERTEX_WEIGHT = 0.001f;

///////////////////////////////////////////////////////////////////////////////////////////////////
struct SkinVertex
{
	SkinVertex()
		: copyPosition(-1)
		, copyNormal(-1)
	{
	}
	VertexInfluence		influences[MAX_VERTEX_INFLUENCE];
	int					copyPosition;
	int					copyNormal;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class SkinnedMeshFile : public MeshFile
{
	friend class CExporter;

public:
	SkinnedMeshFile();

	virtual unsigned long getStaticStreamFormat() const;
	virtual unsigned long getDynamicStreamFormat() const;

	const VECTOR(STRING)& getBoneNames() const;
	const VECTOR(float)& getBoneMaxDistances() const;
	const VECTOR(Matrix)& getOffsetMatrices() const;

	const VECTOR(SkinVertex)& getSkinVertices() const;

	virtual bool importFrom(std::istream& input);

	float getWeightLodError() const;

	size_t getUniquePosCount() const;

private:
	bool importCompressedSkinVertex(std::istream& input, unsigned char* buffer);

	bool unpackVertex(std::istream& input, SkinVertex& vertex, unsigned char* &buffer);

private:
	void clear();

private:
	VECTOR(STRING)		m_boneNames;
	VECTOR(float)		m_boneMaxDistances;
	VECTOR(Matrix)		m_offsetMatrices;
	VECTOR(SkinVertex)	m_skinVertices;
	float				m_weightLodError;	//error caused by ignoring weights other than the 1st one
	size_t				m_uniquePosCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline unsigned long SkinnedMeshFile::getStaticStreamFormat() const
{
	return m_vertexFormat & (TEXCOORD | TEXCOORD2 | COLOR);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline unsigned long SkinnedMeshFile::getDynamicStreamFormat() const
{
	return m_vertexFormat & (POSITION | NORMAL | TANGENT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(SkinVertex)& SkinnedMeshFile::getSkinVertices() const
{
	return m_skinVertices;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(STRING)& SkinnedMeshFile::getBoneNames() const
{
	return m_boneNames;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(float)& SkinnedMeshFile::getBoneMaxDistances() const
{
	return m_boneMaxDistances;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(Matrix)& SkinnedMeshFile::getOffsetMatrices() const
{
	return m_offsetMatrices;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float SkinnedMeshFile::getWeightLodError() const
{
	return m_weightLodError;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t SkinnedMeshFile::getUniquePosCount() const
{
	return m_uniquePosCount;
}

}

#endif
