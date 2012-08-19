#include "Precompiled.h"
#include "SkinnedMeshFile.h"
#include "ChunkFileIo.h"
#include "IMesh.h"
#include "Performance.h"

namespace grp
{

static const int CURRENT_VERSION = 0x0100;

///////////////////////////////////////////////////////////////////////////////////////////////////
SkinnedMeshFile::SkinnedMeshFile()
	: m_weightLodError(0.0f)
	, m_uniquePosCount(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SkinnedMeshFile::importFrom(std::istream& input)
{
	//PERF_NODE_FUNC();

	clear();

	if (!MeshFile::importMesh(input))
	{
		return false;
	}
	size_t skinSize;
	if (!findChunk(input, 'SKIN', skinSize))
	{
		return false;
	}

	//bones
	size_t allBoneSizeLeft;
	size_t boneCount;
	if (!findChunk(input, 'BONS', allBoneSizeLeft, skinSize))
	{
		return false;
	}
	skinSize -= allBoneSizeLeft;
	input.read((char*)&boneCount, sizeof(boneCount));
	if (boneCount <= 0)
	{
		return false;
	}
	allBoneSizeLeft -= sizeof(boneCount);

	m_boneNames.resize(boneCount);
	m_boneMaxDistances.resize(boneCount);
	m_offsetMatrices.resize(boneCount);
	for (size_t i = 0; i < boneCount; ++i)
	{
		size_t boneSizeLeft;
		if (!findChunk(input, 'BONE', boneSizeLeft, allBoneSizeLeft))
		{
			return false;
		}
		allBoneSizeLeft -= (boneSizeLeft + CHUNK_HEADER_SIZE);
		if (!readStringChunk(input, m_boneNames[i], boneSizeLeft, 'NAME'))
		{
			return false;
		}
		for (int row = 0; row < 4; ++row)
		{
			input.read((char*)m_offsetMatrices[i].M[row], sizeof(float) * 3);
		}
		m_offsetMatrices[i]._14 = 0.0f;
		m_offsetMatrices[i]._24 = 0.0f;
		m_offsetMatrices[i]._34 = 0.0f;
		m_offsetMatrices[i]._44 = 1.0f;
	}
	size_t maxDistanceSize;
	if (findChunk(input, 'MAXD', maxDistanceSize, allBoneSizeLeft))
	{
		if (maxDistanceSize != boneCount * sizeof(float))
		{
			return false;
		}
		input.read((char*)&m_boneMaxDistances[0], boneCount * sizeof(float));
	}
	else
	{
		memset(&m_boneMaxDistances[0], 0, boneCount * sizeof(float));
	}

	//skin vertices
	size_t vertexSize;
	if (m_vertexCount <= 0)
	{
		return false;
	}
	m_skinVertices.resize(m_vertexCount);
	bool compressed;

	if (findChunk(input, 'CVTX', vertexSize, skinSize))
	{
		compressed = true;
	}
	else if (findChunk(input, 'VETX', vertexSize, skinSize))
	{
		compressed = false;
	}
	else
	{
		return false;
	}
	skinSize -= (vertexSize + CHUNK_HEADER_SIZE);

	if (compressed)
	{
		VECTOR(unsigned char) buffer(vertexSize);
		input.read((char*)&buffer[0], vertexSize);
		if (!importCompressedSkinVertex(input, &buffer[0]))
		{
			return false;
		}
	}
	else
	{
		if (vertexSize != m_vertexCount * sizeof(SkinVertex))
		{
			return false;
		}
		input.read((char*)&m_skinVertices[0], vertexSize);
	}
	if (!readChunk(input, 'WTER', (char*)&m_weightLodError, sizeof(m_weightLodError)))
	{
		m_weightLodError = 0.0f;
	}
	for (size_t i = 0; i < m_vertexCount; ++i)
	{
		if (m_skinVertices[i].copyPosition >= 0)
		{
			m_uniquePosCount = i;
			break;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SkinnedMeshFile::clear()
{
	m_skinVertices.clear();
	m_boneNames.clear();
	m_offsetMatrices.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SkinnedMeshFile::importCompressedSkinVertex(std::istream& input, unsigned char* buffer)
{
	//PERF_NODE_FUNC();

	for (size_t i = 0; i < m_skinVertices.size(); ++i)
	{
		if (!unpackVertex(input, m_skinVertices[i], buffer))
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SkinnedMeshFile::unpackVertex(std::istream& input, SkinVertex& vertex, unsigned char* &buffer)
{
	unsigned char byType = *(buffer++);
	
	int iNumWeight = ((byType & 0xc0) >> 6) + 1;
	for (int i = 0; i < MAX_VERTEX_INFLUENCE; ++i)
	{
		if (i >= iNumWeight)
		{
			vertex.influences[i].boneIndex = 0;
			vertex.influences[i].weight = 0.0f;
		}
		else
		{
			unsigned char byBone = *(buffer++);
			unsigned char byWeight = *(buffer++);
			if (byBone >= m_boneNames.size())
			{
				return false;
			}
			vertex.influences[i].boneIndex = static_cast<unsigned long>(byBone);
			vertex.influences[i].weight = (float)byWeight / UCHAR_MAX; 
		}
	}
	//
	switch (byType & 0x30)
	{
	case 0x20:	//copy normal
		{
			unsigned short aCopy[2];
			//input.read((char*)aCopy, sizeof(aCopy));
			memcpy(aCopy, buffer, sizeof(aCopy));
			buffer += sizeof(aCopy);
			if (aCopy[0] >= m_skinVertices.size()
				|| aCopy[1] >= m_skinVertices.size())
			{
				return false;
			}
			vertex.copyPosition = static_cast<int>(aCopy[0]);
			vertex.copyNormal = static_cast<int>(aCopy[1]);
		}
		break;
	case 0x10:	//copy pos
		{
			unsigned short wCopy = *((unsigned short*)buffer);
			buffer += sizeof(unsigned short);
			if (wCopy >= m_skinVertices.size())
			{
				return false;
			}
			vertex.copyPosition = static_cast<int>(wCopy);
			vertex.copyNormal = -1;
		}
		break;
	case 0x00:	//copy nothing
		{
			vertex.copyPosition = -1;
			vertex.copyNormal = -1;
		}
		break;
	}
	return true;
}

}
