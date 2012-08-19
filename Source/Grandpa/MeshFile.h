#ifndef __GRP_MESH_FILE_H__
#define __GRP_MESH_FILE_H__

#include "ContentFile.h"
#include <vector>

namespace grp
{

struct LodIndices;

class MeshFile : public ContentFile
{
	friend class CExporter;

public:
	MeshFile();
	virtual ~MeshFile();

	unsigned long getType() const;
	bool checkType(unsigned long type) const;

	const STRING& getProperty() const;

	virtual unsigned long getVertexFormat() const;
	bool checkVertexFormat(unsigned long format) const;

	size_t getVertexCount() const;

	virtual unsigned long getStaticStreamFormat() const = 0;
	virtual unsigned long getDynamicStreamFormat() const = 0;

	const unsigned char* getStaticVertexStream() const;
	const unsigned char* getDynamicVertexStream() const;

	const Matrix& getTransform() const;

	size_t getBufferCount() const;

	size_t getLodVertexCount(float tolerance) const;
	const VECTOR(Index32)& getBufferLodIndices(size_t bufferIndex, float tolerance) const;

	virtual bool importFrom(std::istream& input) = 0;

	bool importMesh(std::istream& input, size_t* outFileSizeLeft = NULL);

	static size_t calculateVertexStride(unsigned long format);

	static size_t getDataOffset(unsigned long format, unsigned long field);

protected:
	void importPosition(std::istream& input, unsigned char* positionPtr, size_t stride);
	void importCompressedPosition(std::istream& input, unsigned char* positionPtr, size_t stride);

	void importNormal(std::istream& input, unsigned char* normalPointer, size_t stride);
	void importCompressedNormal(std::istream& input, unsigned char* normalPointer, size_t stride);

	void importTexCoord(std::istream& input, unsigned char* texCoordPtr, size_t stride);
	void importCompressedTexCoord(std::istream& input, unsigned char* texCoordPtr, size_t stride);

	void importColor(std::istream& input, unsigned char* colorPtr, size_t stride);

	void importCompressedIndex(std::istream& input, Index32* indices, size_t count);


	void unpackPosition(const unsigned short* packed, Vector3& position, const Vector3& minPosition, const Vector3& offset) const;

	void unpackNormal16(unsigned short packed, Vector3& normal) const;

	const LodIndices& findLodIndices(const VECTOR(LodIndices)& buffer, float tolerance) const;

protected:
	unsigned long	m_type;
	STRING			m_property;
	unsigned long	m_vertexFormat;
	size_t			m_vertexCount;

	Matrix			m_transform;

	unsigned char*	m_staticVertexStream;
	unsigned char*	m_dynamicVertexStream;

	VECTOR(VECTOR(LodIndices))	m_meshBuffers;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
struct LodIndices
{
	float			maxError;
	Index32			maxIndex;
	VECTOR(Index32)	indices;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline unsigned long MeshFile::getType() const
{
	return m_type;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const STRING& MeshFile::getProperty() const
{
	return m_property;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool MeshFile::checkType(unsigned long type) const
{
	return ((m_type & type) != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool MeshFile::checkVertexFormat(unsigned long format) const
{
	return ((m_vertexFormat & format) != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline unsigned long MeshFile::getVertexFormat() const
{
	return m_vertexFormat;
}
	
///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t MeshFile::getVertexCount() const
{
	return m_vertexCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const unsigned char* MeshFile::getStaticVertexStream() const
{
	return m_staticVertexStream;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const unsigned char* MeshFile::getDynamicVertexStream() const
{
	return m_dynamicVertexStream;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Matrix& MeshFile::getTransform() const
{
	return m_transform;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t MeshFile::getBufferCount() const
{
	return m_meshBuffers.size();
}

#ifdef GRANDPA_SQRT_TABLE
extern unsigned char g_sqrtTable[256][256];
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void MeshFile::unpackNormal16(unsigned short packed, Vector3& normal) const
{
	unsigned long maxValue = (1 << 8) - 1;
	unsigned long x = (packed & (maxValue << 8)) >> 8;
	unsigned long y = packed & maxValue;
	normal.X = ((float)x / maxValue - 0.5f) * 2.0f;
	normal.Y = ((float)y / maxValue - 0.5f) * 2.0f;
#ifdef GRANDPA_SQRT_TABLE
	normal.Z = g_sqrtTable[x][y] / 255.0f;
#else
	float fx = x / 127.5f - 1.0f;
	float fy = y / 127.5f - 1.0f;
	float sqr = 1.0f - fx * fx - fy * fy;
	if (sqr > 0.0f)
	{
		normal.Z = sqrtf(sqr);
	}
	else
	{
		normal.Z = 0.0f;
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void MeshFile::unpackPosition(const unsigned short* packed,
									  Vector3& position,
									  const Vector3& minPosition,
									  const Vector3& offset) const
{
	position.X = minPosition.X + (offset.X * packed[0]) / USHRT_MAX;
	position.Y = minPosition.Y + (offset.Y * packed[1]) / USHRT_MAX;
	position.Z = minPosition.Z + (offset.Z * packed[2]) / USHRT_MAX;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const LodIndices& MeshFile::findLodIndices(const VECTOR(LodIndices)& buffer, float tolerance) const
{
	for (int i = static_cast<int>(buffer.size()) - 1; i >= 0; --i)
	{
		const LodIndices& indices = buffer[i];
		if (indices.maxError <= tolerance)
		{
			return indices;
		}
	}
	return buffer[0];
}

}

#endif
