#include "Precompiled.h"
#include "SkinnedMesh.h"
#include "ContentResource.h"
#include "Performance.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
SkinnedMesh::SkinnedMesh(const SkinnedMeshResource* resource)
	: Mesh(resource)
	, m_resource(resource)
	, m_updateMode(UPDATE_DEFAULT)
	, m_gpuSkinning(false)
	, m_weightLod(true)
{
	assert(resource != NULL);
	resource->grab();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
SkinnedMesh::~SkinnedMesh()
{
	//if gpu skinning, m_dynamicStream.buffer is only a reference to m_resource->getDynamicVertexStream()
	if (!m_gpuSkinning && m_dynamicStream.buffer != NULL)
	{
		GRP_DELETE(m_dynamicStream.buffer);
	}
	assert(m_resource != NULL);
	m_resource->drop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const Resource* SkinnedMesh::getMeshResource() const
{
	return m_resource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SkinnedMesh::update()
{
	PERF_NODE_FUNC();

	assert(m_finalBoneTransforms.size() == m_boneTransforms.size());
	{
		PERF_NODE("UpdateMatrix");

		assert(m_resource != NULL);
		const VECTOR(Matrix)& offsetMatrices = m_resource->getOffsetMatrices();
		for (size_t i = 0; i < m_finalBoneTransforms.size(); ++i)
		{
			if (m_boneTransforms[i] == NULL)
			{
				m_finalBoneTransforms[i] = offsetMatrices[i];
			}
			else
			{
				offsetMatrices[i].multiply_optimized(*m_boneTransforms[i], m_finalBoneTransforms[i]);
			}
		}
	}
	if (m_gpuSkinning)
	{
		return;
	}
	if (!checkVertexFormat(NORMAL) || m_updateMode == UPDATE_POS_ONLY)
	{
		updateVertex_PosOnly();
	}
	else if (!checkVertexFormat(TANGENT) || m_updateMode == UPDATE_NO_TANGENT)
	{
		updateVertex_NoTangent();
	}
	else
	{
		updateVertex();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SkinnedMesh::updateVertex()
{
	assert(m_resource != NULL);
	const VECTOR(SkinVertex)& skinVertices = m_resource->getSkinVertices();
	const unsigned char* srcStream = m_resource->getDynamicVertexStream();
	
	const unsigned long format = (POSITION | NORMAL | TANGENT);
	assert(m_dynamicStream.format == format);
	size_t stride = m_dynamicStream.stride;

	const unsigned char* srcPositions = srcStream + MeshFile::getDataOffset(format, POSITION);
	const unsigned char* srcNormals = srcStream + MeshFile::getDataOffset(format, NORMAL);
	const unsigned char* srcTangents = srcStream + MeshFile::getDataOffset(format, TANGENT);
	const unsigned char* srcBinormals = srcTangents + sizeof(Vector3);

	assert(m_dynamicStream.buffer != NULL);
	unsigned char* dstPositions = m_dynamicStream.buffer + MeshFile::getDataOffset(format, POSITION);
	unsigned char* dstNormals = m_dynamicStream.buffer + MeshFile::getDataOffset(format, NORMAL);
	unsigned char* dstTangents = m_dynamicStream.buffer + MeshFile::getDataOffset(format, TANGENT);
	unsigned char* dstBinormals = dstTangents + sizeof(Vector3);
	const unsigned char* dstPositionStart = dstPositions;	//backup for further usage
	const unsigned char* dstNormalStart = dstNormals;

	bool oneWeightOnly = (m_weightLod && m_lodTolerance > m_resource->getWeightLodError());
	for (size_t i = 0;
		i < m_vertexCount;
		++i,
		srcPositions += stride, srcNormals += stride, srcTangents += stride, srcBinormals += stride,
		dstPositions += stride, dstNormals += stride, dstTangents += stride, dstBinormals += stride)
	{
		const SkinVertex& vertexCore = skinVertices[i];

		Vector3& dstPosition = *((Vector3*)dstPositions);
		Vector3& dstNormal = *((Vector3*)dstNormals);
		Vector3& dstTangent = *((Vector3*)dstTangents);
		Vector3& dstBinormal = *((Vector3*)dstBinormals);
		const Vector3& srcPosition = *((const Vector3*)srcPositions);
		const Vector3& srcNormal = *((const Vector3*)srcNormals);
		const Vector3& srcTangent = *((const Vector3*)srcTangents);
		const Vector3& srcBinormal = *((const Vector3*)srcBinormals);

		if (oneWeightOnly || vertexCore.influences[0].weight > 0.999f)
		{
			const Matrix& transform = m_finalBoneTransforms[vertexCore.influences[0].boneIndex];
			dstTangent = transform.rotateVector3(srcTangent);
			dstBinormal = transform.rotateVector3(srcBinormal);
			if (vertexCore.copyNormal < 0)
			{
				dstNormal = transform.rotateVector3(srcNormal);
			}
			if (vertexCore.copyPosition < 0)
			{
				dstPosition = transform.transformVector3(srcPosition);
			}
		}
		else
		{
			Matrix totalTransform;
			for (int j = 0; j < MAX_VERTEX_INFLUENCE; ++j)
			{
				//for each bone that influence
				const VertexInfluence& influence = vertexCore.influences[j];
				if (influence.weight < MIN_VERTEX_WEIGHT)
				{
					break;	//sorted by weight, so we can do this
				}
				if (j == 0)
				{
					totalTransform = m_finalBoneTransforms[influence.boneIndex] * influence.weight;
				}
				else
				{
					totalTransform += m_finalBoneTransforms[influence.boneIndex] * influence.weight;
				}
			}
			dstTangent = totalTransform.rotateVector3(srcTangent);
			dstBinormal = totalTransform.rotateVector3(srcBinormal);
			if (vertexCore.copyNormal < 0)
			{
				dstNormal = totalTransform.rotateVector3(srcNormal);
			}
			if (vertexCore.copyPosition < 0)
			{
				dstPosition = totalTransform.transformVector3(srcPosition);
			}
		}

		if (vertexCore.copyPosition >= 0)
		{
			dstPosition = *((const Vector3*)(dstPositionStart + vertexCore.copyPosition * stride));
		}
		if (vertexCore.copyNormal >= 0)
		{
			dstNormal = *((const Vector3*)(dstNormalStart + vertexCore.copyNormal * stride));
		}
		else
		{
			dstNormal.normalize();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SkinnedMesh::updateVertex_NoTangent()
{
	assert(m_resource != NULL);
	const VECTOR(SkinVertex)& skinVertices = m_resource->getSkinVertices();
	const unsigned char* srcStream = m_resource->getDynamicVertexStream();
	
	const unsigned long format = m_dynamicStream.format;
	size_t stride = m_dynamicStream.stride;

	const unsigned char* srcPositions = srcStream + MeshFile::getDataOffset(format, POSITION);
	const unsigned char* srcNormals = srcStream + MeshFile::getDataOffset(format, NORMAL);

	assert(m_dynamicStream.buffer != NULL);
	unsigned char* dstPositions = m_dynamicStream.buffer + MeshFile::getDataOffset(format, POSITION);
	unsigned char* dstNormals = m_dynamicStream.buffer + MeshFile::getDataOffset(format, NORMAL);
	const unsigned char* dstPositionStart = dstPositions;	//backup for further usage
	const unsigned char* dstNormalStart = dstNormals;

	bool oneWeightOnly = (m_weightLod && m_lodTolerance > m_resource->getWeightLodError());
	for (size_t i = 0;
		i < m_vertexCount;
		++i,
		srcPositions += stride, srcNormals += stride,
		dstPositions += stride, dstNormals += stride)
	{
		const SkinVertex& vertexCore = skinVertices[i];

		Vector3& dstPosition = *((Vector3*)dstPositions);
		Vector3& dstNormal = *((Vector3*)dstNormals);

		if (vertexCore.copyNormal >= 0)
		{
			dstPosition = *((const Vector3*)(dstPositionStart + vertexCore.copyPosition * stride));
			dstNormal = *((const Vector3*)(dstNormalStart + vertexCore.copyNormal * stride));
			continue;
		}
		const Vector3& srcPosition = *((const Vector3*)srcPositions);
		const Vector3& srcNormal = *((const Vector3*)srcNormals);
		if (oneWeightOnly || vertexCore.influences[0].weight > 0.999f)
		{
			const Matrix& transform = m_finalBoneTransforms[vertexCore.influences[0].boneIndex];
			dstNormal = transform.rotateVector3(srcNormal);
			if (vertexCore.copyPosition < 0)
			{
				dstPosition = transform.transformVector3(srcPosition);
			}
		}
		else
		{
			for (int j = 0; j < MAX_VERTEX_INFLUENCE; ++j)
			{
				const VertexInfluence& influence = vertexCore.influences[j];
				if (influence.weight < MIN_VERTEX_WEIGHT)
				{
					break;
				}
				assert(influence.boneIndex < m_finalBoneTransforms.size());
				const Matrix& transform = m_finalBoneTransforms[influence.boneIndex];
				if (j == 0)
				{
					dstNormal = (transform.rotateVector3(srcNormal) * influence.weight);
				}
				else
				{
					dstNormal += (transform.rotateVector3(srcNormal) * influence.weight);
				}
				if (vertexCore.copyPosition < 0)
				{
					if (j == 0)
					{
						dstPosition = (transform.transformVector3(srcPosition) * influence.weight);
					}
					else
					{
						dstPosition += (transform.transformVector3(srcPosition) * influence.weight);
					}
				}
			}
		}
		dstNormal.normalize();

		if (vertexCore.copyPosition >= 0)
		{
			dstPosition = *((const Vector3*)(dstPositionStart + vertexCore.copyPosition * stride));
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SkinnedMesh::updateVertex_PosOnly()
{
	assert(m_resource != NULL);
	const VECTOR(SkinVertex)& skinVertices = m_resource->getSkinVertices();
	const unsigned char* srcStream = m_resource->getDynamicVertexStream();
	
	const unsigned long format = m_dynamicStream.format;
	size_t stride = m_dynamicStream.stride;

	const unsigned char* srcPositions = srcStream + MeshFile::getDataOffset(format, POSITION);

	assert(m_dynamicStream.buffer != NULL);
	unsigned char* dstPositions = m_dynamicStream.buffer + MeshFile::getDataOffset(format, POSITION);
	const unsigned char* dstPositionStart = dstPositions;	//backup for further usage

	bool oneWeightOnly = (m_weightLod && m_lodTolerance > m_resource->getWeightLodError());
	//assert(m_vertexCount == m_resource->getVertexCount());
	for (size_t i = 0;	i < m_vertexCount; ++i, srcPositions += stride, dstPositions += stride)
	{
		const SkinVertex& vertexCore = skinVertices[i];

		Vector3& dstPosition = *((Vector3*)dstPositions);
		if (vertexCore.copyPosition >= 0)
		{
			dstPosition = *((const Vector3*)(dstPositionStart + vertexCore.copyPosition * stride));
			continue;
		}
		const Vector3& srcPosition = *((const Vector3*)srcPositions);
		if (oneWeightOnly || vertexCore.influences[0].weight > 0.999f)
		{
			const Matrix& transform = m_finalBoneTransforms[vertexCore.influences[0].boneIndex];
			dstPosition = transform.transformVector3(srcPosition);
		}
		else
		{
			dstPosition = Vector3::ZERO;
			for (int j = 0; j < MAX_VERTEX_INFLUENCE; ++j)
			{
				const VertexInfluence& influence = vertexCore.influences[j];
				if (influence.weight < MIN_VERTEX_WEIGHT)
				{
					break;
				}
				assert(influence.boneIndex < m_finalBoneTransforms.size());
				const Matrix& transform = m_finalBoneTransforms[influence.boneIndex];
				dstPosition += (transform.transformVector3(srcPosition) * influence.weight);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SkinnedMesh::build()
{
	PERF_NODE_FUNC();

	Mesh::build();

	assert(m_resource->getResourceState() == RES_STATE_COMPLETE);

	size_t vertexCount = m_resource->getVertexCount();

	m_staticStream.format = m_resource->getStaticStreamFormat();
	m_staticStream.stride = MeshFile::calculateVertexStride(m_staticStream.format);
	m_staticStream.buffer = const_cast<unsigned char*>(m_resource->getStaticVertexStream());

	m_dynamicStream.format = m_resource->getDynamicStreamFormat();
	m_dynamicStream.stride = MeshFile::calculateVertexStride(m_dynamicStream.format);
	if (m_gpuSkinning)
	{
		m_dynamicStream.buffer = const_cast<unsigned char*>(m_resource->getDynamicVertexStream());
	}
	else
	{
		m_dynamicStream.buffer = GRP_NEW unsigned char[vertexCount * m_dynamicStream.stride];
	}

	size_t boneInfluenceCount = m_resource->getBoneNames().size();
	m_boneTransforms.resize(boneInfluenceCount, NULL);
	m_boneIds.resize(boneInfluenceCount, -1);
	m_finalBoneTransforms.resize(boneInfluenceCount);

	setBuilt();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t SkinnedMesh::getSkinVertexCount() const
{
	if (m_resource != NULL)
	{
		return m_resource->getVertexCount();
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const VertexInfluence* SkinnedMesh::getVertexInfluences(size_t vertexIndex) const
{
	if (m_resource == NULL)
	{
		return NULL;
	}
	assert(vertexIndex < m_resource->getVertexCount());
	return &m_resource->getSkinVertices()[vertexIndex].influences[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t SkinnedMesh::getBBVertexCount() const
{
	if (m_resource != NULL)
	{
		return std::min(m_vertexCount, m_resource->getUniquePosCount());
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SkinnedMesh::setGpuSkinning(bool on)
{
	if (on == m_gpuSkinning)
	{
		return;
	}
	m_gpuSkinning = on;

	if (!isBuilt())
	{
		return;
	}
	assert(m_resource != NULL && m_resource->getResourceState() == RES_STATE_COMPLETE);
	if (on)
	{
		if (m_dynamicStream.buffer != NULL)
		{
			GRP_DELETE(m_dynamicStream.buffer);
		}
		m_dynamicStream.buffer = const_cast<unsigned char*>(m_resource->getDynamicVertexStream());
	}
	else
	{
		m_dynamicStream.buffer = GRP_NEW unsigned char[m_resource->getVertexCount() * m_dynamicStream.stride];
	}
}

}
