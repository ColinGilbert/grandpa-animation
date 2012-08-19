#include "Precompiled.h"
#include "SkinnedMeshExporter.h"
#include "ChunkFileIo.h"
#include "IMesh.h"

namespace grp
{

static const int CURRENT_VERSION = 0x0100;

///////////////////////////////////////////////////////////////////////////////////////////////////
SkinnedMeshExporter::SkinnedMeshExporter()
	: m_weightLodError(0.0f)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SkinnedMeshExporter::exportTo(std::ostream& output,
									bool compressePos,
									bool compressNormal,
									bool compressTexcoord,
									bool compressWeight) const
{
	//MESH
	//SKIN
	//	BONS
	//		BONE
	//			NAME
	//	VETX
	//	TEXC
	//	COLR
	//	INDS
	//		BUFR
	//	WTER

	size_t fileChunkSize;

	if (!MeshExporter::exportMesh(output, &fileChunkSize, compressePos, compressNormal, compressTexcoord))
	{
		return false;
	}

	if (!createChunk(output, 'SKIN'))
	{
		return false;
	}
	size_t skinSize = 0;

	//bone
	size_t boneCount = m_boneNames.size();
	if (boneCount <= 0)
	{
		return false;
	}
	if (!createChunk(output, 'BONS', sizeof(boneCount), (const char*)&boneCount))
	{
		return false;
	}
	size_t allBoneSize = sizeof(boneCount);
	for (size_t i = 0; i < boneCount; ++i)
	{
		if (!createChunk(output, 'BONE'))
		{
			return false;
		}
		size_t nameLength;
		if (!writeStringChunk(output, m_boneNames[i], nameLength, 'NAME'))
		{
			return false;
		}
		size_t boneSize = (nameLength + CHUNK_HEADER_SIZE);
		for (int row = 0; row < 4; ++row)
		{
			output.write((char*)m_offsetMatrices[i].M[row], sizeof(float) * 3);
		}
		boneSize += (sizeof(float) * 3 * 4);
		if (!updateChunkSize(output, boneSize))
		{
			return false;
		}
		allBoneSize += (boneSize + CHUNK_HEADER_SIZE);
	}
	if (!updateChunkSize(output, allBoneSize))
	{
		return false;
	}
	skinSize += (allBoneSize + CHUNK_HEADER_SIZE);
	//skin vertices
	size_t vertexCount = m_skinVertices.size();
	if (vertexCount <= 0 || vertexCount != m_positions.size())
	{
		return false;
	}
	size_t vertexSize;
	if (compressWeight && vertexCount < 0xffff && boneCount <= 256)
	{
		if (!createChunk(output, 'CVTX'))
		{
			return false;
		}
		vertexSize = exportCompressedSkinVertex(output);
	}
	else
	{
		if (!createChunk(output, 'VETX'))
		{
			return false;
		}
		output.write((char*)&m_skinVertices[0], vertexCount * sizeof(SkinVertex));
		vertexSize = vertexCount * sizeof(SkinVertex);
	}
	if (!updateChunkSize(output, vertexSize))
	{
		return false;
	}
	skinSize += (vertexSize + CHUNK_HEADER_SIZE);
	if (m_weightLodError > 0.0f)
	{
		if (!createChunk(output, 'WTER', sizeof(m_weightLodError), (char*)&m_weightLodError))
		{
			return false;
		}
		skinSize += (sizeof(m_weightLodError) + CHUNK_HEADER_SIZE);
	}
	if (!updateChunkSize(output, skinSize))
	{
		return false;
	}
	fileChunkSize += (skinSize + CHUNK_HEADER_SIZE);
	//if (!updateChunkSize(output, fileChunkSize))
	//{
	//	return false;
	//}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SkinnedMeshExporter::clear()
{
	m_skinVertices.clear();
	m_boneNames.clear();
	m_offsetMatrices.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t SkinnedMeshExporter::exportCompressedSkinVertex(std::ostream& output) const
{
	if (m_skinVertices.empty())
	{
		return 0;
	}
	size_t totalSize = 0;
	for (size_t i = 0; i < m_skinVertices.size(); ++i)
	{
		totalSize += packVertex(output, m_skinVertices[i]);
	}
	return totalSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t SkinnedMeshExporter::packVertex(std::ostream& output, const SkinVertex& vertex) const
{
	size_t totalSize = 0;
	//influence个数:2bit；copy pos,copy normal占2bit
	unsigned char byType = 0;
	//冗余标记
	if (vertex.copyNormal >= 0)
	{
		byType |= 0x20;	//00100000;
	}
	else if (vertex.copyPosition >= 0)
	{
		byType |= 0x10;	//00010000;
	}
	//权重
	unsigned char aInflu[8];
	unsigned char byWeightTotal = 0;
	int i;
	for (i = 0; i < MAX_VERTEX_INFLUENCE; ++i)
	{
		aInflu[i * 2] = static_cast<unsigned char>(vertex.influences[i].boneIndex);
		unsigned char byWeight = static_cast<unsigned char>(vertex.influences[i].weight * UCHAR_MAX);
		aInflu[i * 2 + 1] = byWeight;
		byWeightTotal += byWeight;
		if (0 == byWeight)
		{
			break;
		}
	}
	//保证总权重为UCHAR_MAX，总和不足的部分加到第一个权重
	if (byWeightTotal < 0xff)
	{
		aInflu[1] += (0xff - byWeightTotal);
	}
	int iNumWeight = std::max(i, 1);
	byType |= ((iNumWeight - 1) << 6);	//2bit最多记到3，而且不允许一个weight也没有，所以减1再保存

	output.write((char*)(&byType), sizeof(unsigned char));
	totalSize += 1;
	output.write((char*)(aInflu), 2 * iNumWeight);
	totalSize += (2 * iNumWeight);
	if (vertex.copyNormal >= 0)
	{
		unsigned short aCopy[2];
		aCopy[0] = static_cast<unsigned short>(vertex.copyPosition);
		aCopy[1] = static_cast<unsigned short>(vertex.copyNormal);
		output.write((char*)aCopy, sizeof(aCopy));
		totalSize += sizeof(aCopy);
	}
	else if (vertex.copyPosition >= 0)
	{
		unsigned short wCopy = static_cast<unsigned short>(vertex.copyPosition);
		output.write((char*)(&wCopy), sizeof(unsigned short));
		totalSize += sizeof(wCopy);
	}
	return totalSize;
}

}
