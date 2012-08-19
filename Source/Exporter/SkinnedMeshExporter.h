#ifndef __GRP_SKINNED_MESH_EXPORTER_H__
#define __GRP_SKINNED_MESH_EXPORTER_H__

#include "MeshExporter.h"
#include <vector>

namespace grp
{

const size_t MAX_VERTEX_INFLUENCE = 4;
const float MIN_VERTEX_WEIGHT = 0.001f;

///////////////////////////////////////////////////////////////////////////////////////////////////
struct VertexInfluence
{
	VertexInfluence()
		: weight(0.0f)
		, boneIndex(0xffffffff)
	{
	}

	//for sorting
	bool operator< (const VertexInfluence& other) const
	{
		return weight > other.weight;
	}

	unsigned long	boneIndex;	//index，not bone id in skeleton
	float			weight;
};

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
class SkinnedMeshExporter : public MeshExporter
{
	friend class CExporter;

public:
	SkinnedMeshExporter();

	const VECTOR(STRING)& getBoneNames() const;
	const VECTOR(Matrix)& getOffsetMatrices() const;

	const VECTOR(SkinVertex)& getSkinVertices() const;

	virtual bool exportTo(std::ostream& output, bool compressePos, bool compressNormal,
						   bool compressTexcoord, bool compressWeight) const;

	float getWeightLodError() const;

    virtual STRING getTypeDesc() { return L"skinned"; }

private:
	size_t exportCompressedSkinVertex(std::ostream& output) const;

	size_t packVertex(std::ostream& output, const SkinVertex& vertex) const;

private:
	void clear();

private:
	VECTOR(STRING)		m_boneNames;
	VECTOR(Matrix)		m_offsetMatrices;
	VECTOR(SkinVertex)	m_skinVertices;
	float				m_weightLodError;	//忽略第一个以外的顶点weight引起的最大误差
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(SkinVertex)& SkinnedMeshExporter::getSkinVertices() const
{
	return m_skinVertices;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(STRING)& SkinnedMeshExporter::getBoneNames() const
{
	return m_boneNames;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(Matrix)& SkinnedMeshExporter::getOffsetMatrices() const
{
	return m_offsetMatrices;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float SkinnedMeshExporter::getWeightLodError() const
{
	return m_weightLodError;
}

}

#endif
