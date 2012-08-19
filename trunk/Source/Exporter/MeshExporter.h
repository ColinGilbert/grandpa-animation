#ifndef __GRP_MESH_EXPORTER_H__
#define __GRP_MESH_EXPORTER_H__

#include <vector>

namespace grp
{

struct LodIndices;

class MeshExporter
{
	friend class CExporter;

public:
	MeshExporter();

	virtual bool exportTo(std::ostream& output, bool compressePos, bool compressNormal,
							bool compressTexcoord, bool compressWeight) const = 0;

    virtual STRING getTypeDesc() = 0;

	unsigned long getType() const;
	bool checkType(unsigned long type) const;

	unsigned long getVertexFormat() const;
	bool checkVertexFormat(unsigned long format) const;

	const STRING& getName() const;

	const Matrix& getTransform() const;

	size_t getBufferCount() const;

	const Vector3* getPositions() const;
	const Vector3* getNormals() const;
	const Vector3* getTangents() const;
	const Vector3* getBinormals() const;

	const Color32* getColors() const;

	size_t getTexcoordLayerCount() const;
	const Vector2* getTexCoords(size_t index) const;

	bool exportMesh(std::ostream& output, size_t* outFileSize, bool compressePos,
					 bool compressNormal, bool compressTexcoord) const;

protected:
	size_t exportCompressedPosition(std::ostream& output) const;

	size_t exportCompressedNormal(std::ostream& output, const Vector3* normals, size_t count) const;

	size_t exportCompressedTexCoord(std::ostream& output, const Vector2* texCoordes, size_t count) const;

	size_t exportCompressedIndex(std::ostream& output,	const Index32* indices, size_t count) const;

	void packPosition(unsigned short* packed, const Vector3& position, const Vector3& minPosition, const Vector3& offset) const;

	void packNormal16(unsigned short& packed, const Vector3& normal) const;

protected:
	STRING			m_name;
	STRING			m_property;
	unsigned long	m_type;
	unsigned long	m_vertexFormat;
	Matrix			m_transform;

	VECTOR(Vector3)	m_positions;
	VECTOR(Vector3)	m_normals;
	VECTOR(Vector3)	m_tangents;
	VECTOR(Vector3)	m_binormals;
	VECTOR(size_t)	m_lodVertexCounts;

	VECTOR(VECTOR(Vector2))	m_texCoordsArray;
	VECTOR(Color32)			m_colors;

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
inline unsigned long MeshExporter::getType() const
{
	return m_type;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool MeshExporter::checkType(unsigned long type) const
{
	return ((m_type & type) != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline unsigned long MeshExporter::getVertexFormat() const
{
	return m_vertexFormat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool MeshExporter::checkVertexFormat(unsigned long format) const
{
	return ((m_vertexFormat & format) != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const STRING& MeshExporter::getName() const
{
	return m_name;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Matrix& MeshExporter::getTransform() const
{
	return m_transform;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t MeshExporter::getBufferCount() const
{
	return m_meshBuffers.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Vector3* MeshExporter::getPositions() const
{
	return m_positions.empty() ? NULL : (const Vector3*)&m_positions[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Vector3* MeshExporter::getNormals() const
{
	return m_normals.empty() ? NULL : (const Vector3*)&m_normals[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Vector3* MeshExporter::getTangents() const
{
	return m_tangents.empty() ? NULL : (const Vector3*)&m_tangents[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Vector3* MeshExporter::getBinormals() const
{
	return m_binormals.empty() ? NULL : (const Vector3*)&m_binormals[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t MeshExporter::getTexcoordLayerCount() const
{
	return m_texCoordsArray.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Vector2* MeshExporter::getTexCoords(size_t index) const
{
	assert(index < m_texCoordsArray.size()); 
	return m_texCoordsArray[index].empty() ? NULL : (const Vector2*)&(m_texCoordsArray[index][0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Color32* MeshExporter::getColors() const
{
	return m_colors.empty() ? NULL : (const Color32*)&m_colors[0];
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void MeshExporter::packNormal16(unsigned short& packed, const Vector3& normal) const
{
	unsigned long maxValue = (1 << 8) - 1;
	unsigned long x = (unsigned long)(maxValue * (normal.X + 1.0f) / 2.0f);
	unsigned long y = (unsigned long)(maxValue * (normal.Y + 1.0f) / 2.0f);
	assert(x <= maxValue);
	assert(y <= maxValue);
	packed = (unsigned short)((x << 8) | y);
}

#ifdef GRANDPA_SQRT_TABLE
extern unsigned char g_sqrtTable[256][256];
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void MeshExporter::packPosition(unsigned short* packed,
									const Vector3& position,
									const Vector3& minPosition,
									const Vector3& offset) const
{
	packed[0] = (unsigned short)(USHRT_MAX * (position.X - minPosition.X) / offset.X);
	packed[1] = (unsigned short)(USHRT_MAX * (position.Y - minPosition.Y) / offset.Y);
	packed[2] = (unsigned short)(USHRT_MAX * (position.Z - minPosition.Z) / offset.Z);
}

}

#endif
