#include "Precompiled.h"
#include "MeshFile.h"
#include "IMesh.h"
#include "ChunkFileIo.h"
#include "Performance.h"

namespace grp
{

static const int CURRENT_VERSION = 0x0100;

///////////////////////////////////////////////////////////////////////////////////////////////////
MeshFile::MeshFile()
	: m_type(0)
	, m_vertexFormat(0)
	, m_vertexCount(0)
	, m_transform(Matrix::IDENTITY)
	, m_staticVertexStream(NULL)
	, m_dynamicVertexStream(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
MeshFile::~MeshFile()
{
	if (m_staticVertexStream != NULL)
	{
		GRP_DELETE(m_staticVertexStream);
	}
	if (m_dynamicVertexStream != NULL)
	{
		GRP_DELETE(m_dynamicVertexStream);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t MeshFile::getLodVertexCount(float tolerance) const
{
	if (tolerance == 0.0f)
	{
		return m_vertexCount;
	}
	size_t maxIndex = 0;
	for (VECTOR(VECTOR(LodIndices))::const_iterator bufferIter = m_meshBuffers.begin();
		bufferIter != m_meshBuffers.end();
		++bufferIter)
	{
		const LodIndices& indices = findLodIndices(*bufferIter, tolerance);
		if (indices.maxIndex > maxIndex)
		{
			maxIndex = indices.maxIndex;
		}
	}
	return maxIndex + 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const VECTOR(Index32)& MeshFile::getBufferLodIndices(size_t bufferIndex, float tolerance) const
{
	return findLodIndices(m_meshBuffers[bufferIndex], tolerance).indices;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool MeshFile::importMesh(std::istream& input, size_t* outFileSizeLeft)
{
	//PERF_NODE_FUNC();

	size_t fileSizeLeft;
	if (!findChunk(input, 'MESH', fileSizeLeft))
	{
		return false;
	}

	int version;

	if (fileSizeLeft < sizeof(version) + sizeof(m_type) + sizeof(m_vertexCount))
	{
		return false;
	}
	input.read((char*)&version, sizeof(version));
	fileSizeLeft -= sizeof(version);

	unsigned long type;
	input.read((char*)&type, sizeof(type));
	fileSizeLeft -= sizeof(type);
	m_type = type & 0xff;
	m_vertexFormat = type & 0xffffff00;

	if (checkVertexFormat(TANGENT) && (!checkVertexFormat(NORMAL) || !checkVertexFormat(TEXCOORD))
		|| (checkVertexFormat(TEXCOORD2) && !checkVertexFormat(TEXCOORD)))
	{
		return false;
	}

	input.read((char*)&m_vertexCount, sizeof(m_vertexCount));
	fileSizeLeft -= sizeof(m_vertexCount);

	//name
	if (!readStringChunk(input, m_name, fileSizeLeft, 'NAME'))
	{
		return false;
	}

	unsigned long staticFormat;
	unsigned long dynamicFormat;
	bool isSkinnedMesh = !checkType(MESH_RIGID);
	if (isSkinnedMesh)
	{
		staticFormat = m_vertexFormat & (COLOR | TEXCOORD | TEXCOORD2);
		dynamicFormat = m_vertexFormat & (POSITION | NORMAL | TANGENT);
	}
	else
	{
		staticFormat = m_vertexFormat;
		dynamicFormat = 0;
	}
	size_t staticStride = calculateVertexStride(staticFormat);
	size_t dynamicStride = calculateVertexStride(dynamicFormat);
	if (staticStride > 0)
	{
		m_staticVertexStream = GRP_NEW unsigned char[staticStride * m_vertexCount];
	}
	if (dynamicStride > 0)
	{
		m_dynamicVertexStream = GRP_NEW unsigned char[dynamicStride * m_vertexCount];
	}
	if (m_staticVertexStream == NULL && m_dynamicVertexStream == NULL)
	{
		return false;
	}

	//position
	unsigned char* positionStart;
	size_t positionStride;
	if (isSkinnedMesh)
	{
		positionStart = m_dynamicVertexStream + getDataOffset(dynamicFormat, POSITION);
		positionStride = dynamicStride;
	}
	else
	{
		positionStart = m_staticVertexStream + getDataOffset(staticFormat, POSITION);
		positionStride = staticStride;
	}
	size_t positionSize;
	if (findChunk(input, 'CPOS', positionSize, fileSizeLeft))
	{
		//check positionSize?
		importCompressedPosition(input, positionStart, positionStride);
	}
	else if (findChunk(input, 'POSI', positionSize, fileSizeLeft))
	{
		if (positionSize != m_vertexCount * sizeof(Vector3))
		{
			return false;
		}
		importPosition(input, positionStart, positionStride);
	}
	else
	{
		return false;	//must have position
	}
	fileSizeLeft -= (positionSize + CHUNK_HEADER_SIZE);

	//normal
	if (checkVertexFormat(NORMAL))
	{
		unsigned char* normalStart;
		size_t normalStride;
		if (isSkinnedMesh)
		{
			normalStart = m_dynamicVertexStream + getDataOffset(dynamicFormat, NORMAL);
			normalStride = dynamicStride;
		}
		else
		{
			normalStart = m_staticVertexStream + getDataOffset(staticFormat, NORMAL);
			normalStride = staticStride;
		}
		size_t normalSize;
		if (findChunk(input, 'CNOR', normalSize, fileSizeLeft))
		{
			importCompressedNormal(input, normalStart, normalStride);
		}
		else if (findChunk(input, 'NORM', normalSize, fileSizeLeft))
		{
			if (normalSize != m_vertexCount * sizeof(Vector3))
			{
				return false;
			}
			importNormal(input, normalStart, normalStride);
		}
		else 
		{
			return false;
		}
		fileSizeLeft -= (normalSize + CHUNK_HEADER_SIZE);
	}
	//tangent and binormal
	if (checkVertexFormat(TANGENT))
	{
		unsigned char* tangentStart;
		size_t tangentStride;
		if (isSkinnedMesh)
		{
			tangentStart = m_dynamicVertexStream + getDataOffset(dynamicFormat, TANGENT);
			tangentStride = dynamicStride;
		}
		else
		{
			tangentStart = m_staticVertexStream + getDataOffset(staticFormat, TANGENT);
			tangentStride = staticStride;
		}
		size_t tangentSize;
		if (findChunk(input, 'CTAN', tangentSize, fileSizeLeft))
		{
			importCompressedNormal(input, tangentStart, tangentStride);
			importCompressedNormal(input, tangentStart + sizeof(Vector3), tangentStride);
		}
		else if (findChunk(input, 'TANG', tangentSize, fileSizeLeft))
		{
			if (tangentSize != m_vertexCount * (sizeof(Vector3) + sizeof(Vector3)))
			{
				return false;
			}
			importNormal(input, tangentStart, tangentStride);
			importNormal(input, tangentStart + sizeof(Vector3), tangentStride);
		}
		else 
		{
			return false;
		}
		fileSizeLeft -= (tangentSize + CHUNK_HEADER_SIZE);
	}
	//texCoord
	if (checkVertexFormat(TEXCOORD))
	{
		size_t texCoordSize;
		size_t texCoordCount;
		bool compressed;
		if (findChunk(input, 'CTEX', texCoordSize, fileSizeLeft))
		{
			compressed = true;
		}
		else if (findChunk(input, 'TEXC', texCoordSize, fileSizeLeft))
		{
			compressed = false;
		}
		else
		{
			return false;
		}
		input.read((char*)&texCoordCount, sizeof(texCoordCount));
		if ((checkVertexFormat(TEXCOORD2) && texCoordCount < 2)
			|| (!checkVertexFormat(TEXCOORD2) && texCoordCount != 1))
		{
			return false;
		}
		unsigned char* texcoordStart = m_staticVertexStream + getDataOffset(staticFormat, TEXCOORD);
		if (compressed)
		{
			importCompressedTexCoord(input, texcoordStart, staticStride);
		}
		else
		{
			importTexCoord(input, texcoordStart, staticStride);
		}
		if (checkVertexFormat(TEXCOORD2))
		{
			texcoordStart = m_staticVertexStream + getDataOffset(staticFormat, TEXCOORD2);
			if (compressed)
			{
				importCompressedTexCoord(input, texcoordStart, staticStride);
			}
			else
			{
				importTexCoord(input, texcoordStart, staticStride);
			}
		}
		//more than 2 texcoord layers, discard
		if (texCoordCount > 2)
		{
			input.seekg(m_vertexCount * sizeof(unsigned long) * (texCoordCount - 2), std::ios::cur);
		}
		fileSizeLeft -= (texCoordSize + CHUNK_HEADER_SIZE);
	}
	//color
	if (checkVertexFormat(COLOR))
	{
		size_t colorSize;
		unsigned char* colorStart = m_staticVertexStream + getDataOffset(staticFormat, COLOR);
		if (!findChunk(input, 'COLR', colorSize, fileSizeLeft)
			|| colorSize !=  m_vertexCount * sizeof(Color32))
		{
			return false;
		}
		importColor(input, colorStart, staticStride);
		fileSizeLeft -= (colorSize + CHUNK_HEADER_SIZE);
	}
	//mesh buffers
	size_t bufferCount;
	size_t allBufferSizeLeft;
	if (!findChunk(input, 'BUFS', allBufferSizeLeft, fileSizeLeft))
	{
		return false;
	}
	fileSizeLeft -= (allBufferSizeLeft + CHUNK_HEADER_SIZE);

	if (allBufferSizeLeft < sizeof(bufferCount))
	{
		return false;
	}
	input.read((char*)&bufferCount, sizeof(bufferCount));
	allBufferSizeLeft -= sizeof(bufferCount);

	m_meshBuffers.resize(bufferCount);
	for (size_t bufferIndex = 0; bufferIndex < bufferCount; ++bufferIndex)
	{
		PERF_NODE("read mesh buffers");

		VECTOR(LodIndices)& buffer = m_meshBuffers[bufferIndex];
		size_t lodCount;
		size_t bufferSizeLeft;
		if (!findChunk(input, 'BUFR', bufferSizeLeft, allBufferSizeLeft))
		{
			return false;
		}
		allBufferSizeLeft -= (bufferSizeLeft + CHUNK_HEADER_SIZE);
		if (bufferSizeLeft < sizeof(lodCount))
		{
			return false;
		}
		input.read((char*)&lodCount, sizeof(lodCount));
		bufferSizeLeft -= sizeof(lodCount);
		if (lodCount <= 0)
		{
			return false;
		}
		buffer.resize(lodCount);
		for (size_t i = 0; i < lodCount; ++i)
		{
			LodIndices& lodIndices = buffer[i];
			size_t lodIndicesSize;
			bool compressed;
			if (findChunk(input, 'CIND', lodIndicesSize, bufferSizeLeft))
			{
				compressed = true;
			}
			else if (findChunk(input, 'INDS', lodIndicesSize, bufferSizeLeft))
			{
				compressed = false;
			}
			else
			{
				return false;
			}
			input.read((char*)&(lodIndices.maxError), sizeof(lodIndices.maxError));
			input.read((char*)&(lodIndices.maxIndex), sizeof(lodIndices.maxIndex));
			size_t indexCount = 0;
			input.read((char*)&indexCount, sizeof(indexCount));
			if (indexCount <= 0 || indexCount % 3 != 0)
			{
				return false;
			}
			lodIndices.indices.resize(indexCount);
			if (compressed)
			{
				importCompressedIndex(input, &lodIndices.indices[0], indexCount);
			}
			else
			{
				input.read((char*)&(lodIndices.indices[0]), indexCount * sizeof(Index32));
			}
			bufferSizeLeft -= (lodIndicesSize + CHUNK_HEADER_SIZE);
		}
	}
	if (readChunk(input, 'TSFM', (char*)&m_transform, sizeof(m_transform), fileSizeLeft))
	{
		fileSizeLeft -= (sizeof(m_transform) + CHUNK_HEADER_SIZE);
	}

	readStringChunk(input, m_property, fileSizeLeft, 'PROP');

	if (outFileSizeLeft != NULL)
	{
		*outFileSizeLeft = fileSizeLeft;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshFile::importPosition(std::istream& input, unsigned char* positionPtr, size_t stride)
{
	for (size_t i = 0; i < m_vertexCount; ++i, positionPtr += stride)
	{
		input.read((char*)positionPtr, sizeof(Vector3));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshFile::importCompressedPosition(std::istream& input, unsigned char* positionPtr, size_t stride)
{
	//PERF_NODE_FUNC();

	VECTOR(unsigned short) compressed(3 * m_vertexCount);

	Vector3 minPosition, maxPosition, offset;
	input.read((char*)&minPosition, sizeof(Vector3));
	input.read((char*)&maxPosition, sizeof(Vector3));
	input.read((char*)&compressed[0], 3 * m_vertexCount * sizeof(unsigned short));
	offset = maxPosition - minPosition;

	for (size_t i = 0; i < m_vertexCount; ++i, positionPtr += stride)
	{
		unpackPosition(&(compressed[i * 3]), *((Vector3*)positionPtr), minPosition, offset);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshFile::importNormal(std::istream& input, unsigned char* normalPointer, size_t stride)
{
	for (size_t i = 0; i < m_vertexCount; ++i, normalPointer += stride)
	{
		input.read((char*)normalPointer, sizeof(Vector3));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshFile::importCompressedNormal(std::istream& input, unsigned char* normalPointer, size_t stride)
{
	//PERF_NODE_FUNC();

	VECTOR(unsigned short) compressed(m_vertexCount);
	VECTOR(char) sign((m_vertexCount + 7) / 8);
	input.read(&sign[0], (m_vertexCount + 7) / 8);
	input.read((char*)&compressed[0], m_vertexCount * sizeof(unsigned short));
	for (size_t i = 0; i < m_vertexCount; ++i, normalPointer += stride)
	{
		Vector3& normal = *((Vector3*)normalPointer);
		unpackNormal16(compressed[i], normal);
		if ((sign[i/8] & (1 << (i % 8))) != 0)
		{
			normal.Z = -normal.Z;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshFile::importTexCoord(std::istream& input, unsigned char* texCoordPtr, size_t stride)
{
	for (size_t i = 0; i < m_vertexCount; ++i, texCoordPtr += stride)
	{
		input.read((char*)texCoordPtr, sizeof(Vector2));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshFile::importCompressedTexCoord(std::istream& input, unsigned char* texCoordPtr, size_t stride)
{
	//PERF_NODE_FUNC();

	VECTOR(unsigned long) compressed(m_vertexCount);
	input.read((char*)(&(compressed[0])), m_vertexCount * sizeof(unsigned long));
	for (size_t i = 0; i < m_vertexCount; ++i, texCoordPtr += stride)
	{
		Vector2& texCoord = *((Vector2*)texCoordPtr);
		texCoord.X = (float)(compressed[i] >> 16) / USHRT_MAX;
		texCoord.Y = (float)(compressed[i] & USHRT_MAX) / USHRT_MAX;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshFile::importColor(std::istream& input, unsigned char* colorPtr, size_t stride)
{
	for (size_t i = 0; i < m_vertexCount; ++i, colorPtr += stride)
	{
		input.read((char*)colorPtr, sizeof(Color32));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MeshFile::importCompressedIndex(std::istream& input, Index32* indices, size_t count)
{
	//PERF_NODE_FUNC();

	VECTOR(unsigned short) compressed(count);
	input.read((char*)(&(compressed[0])), count * sizeof(unsigned short));
	for (size_t i = 0; i < count; ++i)
	{
		indices[i] = static_cast<unsigned long>(compressed[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t MeshFile::calculateVertexStride(unsigned long format)
{
	size_t stride = 0;
	if ((format & POSITION) != 0)
	{
		stride += sizeof(Vector3);
	}
	if ((format & NORMAL) != 0)
	{
		stride += sizeof(Vector3);
	}
	if ((format & TANGENT) != 0)
	{
		stride += (2 * sizeof(Vector3));
	}
	if ((format & TEXCOORD) != 0)
	{
		stride += sizeof(Vector2);
	}
	if ((format & TEXCOORD2) != 0)
	{
		stride += sizeof(Vector2);
	}
	if ((format & COLOR) != 0)
	{
		stride += sizeof(Color32);
	}
	return stride;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t MeshFile::getDataOffset(unsigned long format, unsigned long field)
{
	if ((format & field) == 0)
	{
		return 0xffffffff;
	}
	if (field == POSITION)
	{
		return 0;
	}
	if (field == NORMAL)
	{
		return sizeof(Vector3);
	}
	if (field == TANGENT)
	{
		//must have normal if there's tangent
		return sizeof(Vector3) * 2;
	}
	if (field == TEXCOORD)
	{
		if ((format & POSITION) == 0)
		{
			return 0;
		}
		if ((format & NORMAL) == 0)
		{
			return sizeof(Vector3);
		}
		if ((format & TANGENT) == 0)
		{
			return sizeof(Vector3) * 2;
		}
		//must have binormal if there's tangent
		return sizeof(Vector3) * 4;
	}
	if (field == TEXCOORD2)
	{
		if ((format & POSITION) == 0)
		{
			return sizeof(Vector2);
		}
		//must have TEXCOORD if there's TEXCOORD2
		if ((format & NORMAL) == 0)
		{
			return sizeof(Vector3) + sizeof(Vector2);
		}
		if ((format & TANGENT) == 0)
		{
			return sizeof(Vector3) * 2 + sizeof(Vector2);
		}
		//must have binormal if there's tangent
		return sizeof(Vector3) * 4 + sizeof(Vector2);
	}
	return 0xffffffff;
}

}
