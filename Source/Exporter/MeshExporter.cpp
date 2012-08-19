#include "Precompiled.h"
#include "MeshExporter.h"
#include "IMesh.h"
#include "ChunkFileIo.h"

namespace grp
{

static const int CURRENT_VERSION = 0x0100;

///////////////////////////////////////////////////////////////////////////////////////////////////
MeshExporter::MeshExporter()
	: m_type(0)
	, m_transform(Matrix::IDENTITY)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool MeshExporter::exportMesh(std::ostream& output,
							   size_t* outFileSize,
							   bool compressePos,
							   bool compressNormal,
							   bool compressTexcoord) const
{
	//MESH
	//	NAME
	//	POSI
	//	NORM
	//	TANG
	//	TEXC
	//	COLR
	//	BUFS
	//		BUFR	
	//			INDS
	//	TSFM
	if (!createChunk(output, 'MESH', sizeof(CURRENT_VERSION), (const char*)&CURRENT_VERSION))
	{
		return false;
	}
	size_t fileChunkSize = sizeof(CURRENT_VERSION);
	unsigned long type = m_type | m_vertexFormat;
	output.write((char*)&type, sizeof(type));
	fileChunkSize += sizeof(type);

	size_t vertexCount = m_positions.size();
	output.write((char*)&vertexCount, sizeof(vertexCount));
	fileChunkSize += sizeof(vertexCount);

	//name
	size_t nameLength;
	if (!writeStringChunk(output, m_name, nameLength, 'NAME'))
	{
		return false;
	}
	fileChunkSize += (nameLength + CHUNK_HEADER_SIZE);
	//position
	size_t positionSize;
	if (compressePos)
	{
		if (!createChunk(output, 'CPOS'))
		{
			return false;
		}
		positionSize = exportCompressedPosition(output);
		if (!updateChunkSize(output, positionSize))
		{
			return false;
		}
	}
	else
	{
		positionSize = vertexCount * sizeof(Vector3);
		if (!createChunk(output, 'POSI', positionSize, (const char*)&m_positions[0]))
		{
			return false;
		}
	}
	fileChunkSize += (positionSize + CHUNK_HEADER_SIZE);
	//normal
	if (checkVertexFormat(NORMAL))
	{
		size_t normalSize;
		assert(m_normals.size() == vertexCount);
		if (compressNormal)
		{
			if (!createChunk(output, 'CNOR'))
			{
				return false;
			}
			normalSize = exportCompressedNormal(output, (const Vector3*)&m_normals[0], vertexCount);
			if (!updateChunkSize(output, normalSize))
			{
				return false;
			}
		}
		else
		{
			normalSize = vertexCount * sizeof(Vector3);
			if (!createChunk(output, 'NORM', normalSize, (const char*)&m_normals[0]))
			{
				return false;
			}
		}
		fileChunkSize += (normalSize + CHUNK_HEADER_SIZE);
	}
	//tangent, binormal
	if (checkVertexFormat(TANGENT))
	{
		size_t tangentSize;	//including tangent and binormal
		assert(m_tangents.size() == vertexCount);
		assert(m_binormals.size() == vertexCount);
		if (compressNormal)
		{
			if (!createChunk(output, 'CTAN'))
			{
				return false;
			}
			tangentSize = exportCompressedNormal(output, (const Vector3*)&m_tangents[0], vertexCount);
			tangentSize += exportCompressedNormal(output, (const Vector3*)&m_binormals[0], vertexCount);
		}
		else
		{
			tangentSize = vertexCount * (sizeof(Vector3) + sizeof(Vector3));
			if (!createChunk(output, 'TANG'))
			{
				return false;
			}
			output.write((char*)&m_tangents[0], vertexCount * sizeof(Vector3));
			output.write((char*)&m_binormals[0], vertexCount * sizeof(Vector3));
		}
		if (!updateChunkSize(output, tangentSize))
		{
			return false;
		}
		fileChunkSize += (tangentSize + CHUNK_HEADER_SIZE);
	}
	//texCoord
	if (checkVertexFormat(TEXCOORD))
	{
		size_t texCoordCount = m_texCoordsArray.size();
		if (compressTexcoord)
		{
			if (!createChunk(output, 'CTEX', sizeof(texCoordCount), (const char*)&texCoordCount))
			{
				return false;
			}
		}
		else
		{
			if (!createChunk(output, 'TEXC', sizeof(texCoordCount), (const char*)&texCoordCount))
			{
				return false;
			}
		}
		size_t texCoordSize = sizeof(texCoordCount);
		for (size_t i = 0; i < texCoordCount; ++i)
		{
			if (compressTexcoord)
			{
				texCoordSize += exportCompressedTexCoord(output, (const Vector2*)&m_texCoordsArray[i][0], vertexCount);
			}
			else
			{
				output.write((char*)&m_texCoordsArray[i][0], vertexCount * sizeof(Vector2));
				texCoordSize += (vertexCount * sizeof(Vector2));
			}
		}
		if (!updateChunkSize(output, texCoordSize))
		{
			return false;
		}
		fileChunkSize += (texCoordSize + CHUNK_HEADER_SIZE);
	}
	//vertex color
	if (checkVertexFormat(COLOR))
	{
		assert(m_colors.size() == vertexCount);
		size_t colorSize = vertexCount * sizeof(Color32);
		if (!createChunk(output, 'COLR', colorSize, (const char*)&m_colors[0]))
		{
			return false;
		}
		fileChunkSize += (colorSize + CHUNK_HEADER_SIZE);
	}
	//mesh buffers
	size_t bufferCount = m_meshBuffers.size();
	if (!createChunk(output, 'BUFS', sizeof(bufferCount), (const char*)&bufferCount))
	{
		return false;
	}
	size_t allBufferSize = sizeof(bufferCount);
	
	for (size_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex)
	{
		const VECTOR(LodIndices)& buffer = m_meshBuffers[bufferIndex];
		size_t lodCount = buffer.size();
		if (lodCount == 0)
		{
			return false;
		}
		if (!createChunk(output, 'BUFR', sizeof(lodCount), (const char*)&lodCount))
		{
			return false;
		}
		size_t bufferSize = sizeof(lodCount);
		for (size_t i = 0; i < lodCount; ++i)
		{	
			const LodIndices& lodIndices = buffer[i];
			if (lodIndices.maxIndex <= 0xffff)
			{
				if (!createChunk(output, 'CIND', sizeof(lodIndices.maxError), (const char*)&lodIndices.maxError))
				{
					return false;
				}
			}
			else
			{
				if (!createChunk(output, 'INDS', sizeof(lodIndices.maxError), (const char*)&lodIndices.maxError))
				{
					return false;
				}
			}
			size_t lodIndicesSize = sizeof(lodIndices.maxError);
			output.write((char*)&lodIndices.maxIndex, sizeof(lodIndices.maxIndex));
			lodIndicesSize += sizeof(lodIndices.maxIndex);
			size_t indexCount = lodIndices.indices.size();
			if (indexCount <= 0 || indexCount % 3 != 0)
			{
				return false;
			}
			output.write((char*)&indexCount, sizeof(indexCount));
			lodIndicesSize += sizeof(indexCount);

			if (lodIndices.maxIndex <= 0xffff)
			{
				lodIndicesSize += exportCompressedIndex(output, &(lodIndices.indices[0]), indexCount);
			}
			else
			{
				output.write((char*)&(lodIndices.indices[0]), indexCount * sizeof(Index32));
				bufferSize += (indexCount * sizeof(Index32));
			}
			if (!updateChunkSize(output, lodIndicesSize))
			{
				return false;
			}
			bufferSize += (lodIndicesSize + CHUNK_HEADER_SIZE);
		}
		if (!updateChunkSize(output, bufferSize))
		{
			return false;
		}
		allBufferSize += (bufferSize + CHUNK_HEADER_SIZE);
	}
	if (!updateChunkSize(output, allBufferSize))
	{
		return false;
	}
	fileChunkSize += (allBufferSize + CHUNK_HEADER_SIZE);
	
	if (m_transform != Matrix::IDENTITY)
	{
		if (!createChunk(output, 'TSFM', sizeof(m_transform), (char*)&m_transform))
		{
			return false;
		}
		fileChunkSize += (sizeof(m_transform) + CHUNK_HEADER_SIZE);
	}
	if (m_property != L"")
	{
		size_t propertyLength;
		if (!writeStringChunk(output, m_property, propertyLength, 'PROP'))
		{
			return false;
		}
		fileChunkSize += (propertyLength + CHUNK_HEADER_SIZE);
	}
	if (!updateChunkSize(output, fileChunkSize))
	{
		return false;
	}
	if (outFileSize != NULL)
	{
		*outFileSize = fileChunkSize;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t MeshExporter::exportCompressedPosition(std::ostream& output) const
{
	if (m_positions.empty())
	{
		return 0;
	}
	const Vector3* positions = (const Vector3*)&m_positions[0];
	AaBox box(positions[0]);
	size_t vertexCount = m_positions.size();
	for (size_t i = 1; i < vertexCount; ++i)
	{
		box.addInternalPoint(positions[i]);
	}
	Vector3 boxSize = box.getSize();
	//float->unsigned short
	VECTOR(unsigned short) compressed(3 * vertexCount);
	for (size_t i = 0; i < vertexCount; ++i)
	{
		packPosition(&(compressed[i * 3]), positions[i], box.MinEdge, boxSize);
	}
	output.write((char*)&box.MinEdge, sizeof(Vector3));
	output.write((char*)&box.MaxEdge, sizeof(Vector3));
	output.write((char*)&compressed[0], 3 * vertexCount * sizeof(unsigned short));

	return (sizeof(Vector3) + sizeof(Vector3) + 3 * vertexCount * sizeof(unsigned short));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t MeshExporter::exportCompressedNormal(std::ostream& output, const Vector3* normals, size_t count) const
{
	//3个float压缩到一个unsigned short
	VECTOR(unsigned short) compressed(count);
	size_t signSize = (count + 7) / 8;
	VECTOR(char) sign(signSize, 0);
	for (size_t i = 0; i < count; ++i)
	{
		packNormal16(compressed[i], normals[i]);
		if (normals[i].Z < 0.0f)
		{
			sign[i/8] |= (1 << (i % 8));
		}
	}
	output.write(&sign[0], signSize * sizeof(char));
	output.write((char*)&compressed[0], count * sizeof(unsigned short));
	return (signSize * sizeof(char) + count * sizeof(unsigned short));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t MeshExporter::exportCompressedTexCoord(std::ostream& output, const Vector2* texCoords, size_t count) const
{
	//两个float压缩到DWORD
	VECTOR(unsigned long) compressed(count);
	for (size_t i = 0; i < count; ++i)
	{
		const Vector2& texCoord = texCoords[i];
		unsigned long& value = compressed[i];

		unsigned long u = (unsigned long)(USHRT_MAX * texCoord.X);
		unsigned long v = (unsigned long)(USHRT_MAX * texCoord.Y);
		//assert(u <= USHRT_MAX);
		//assert(v <= USHRT_MAX);

		value = (u << 16) | v;
	}
	size_t totalSize = count * sizeof(unsigned long);
	output.write((char*)&compressed[0], totalSize);
	return totalSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t MeshExporter::exportCompressedIndex(std::ostream& output, const Index32* indices, size_t count) const
{
	//DWORD压缩为WORD
	VECTOR(unsigned short) compressed(count);
	for (size_t i = 0; i < count; ++i)
	{
		compressed[i] = static_cast<unsigned short>(indices[i]);
	}
	size_t totalSize = count * sizeof(unsigned short);
	output.write((char*)&compressed[0], totalSize);
	return totalSize;
}

}
