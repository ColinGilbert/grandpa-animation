#include "Precompiled.h"
#include "Mesh.h"
#include "MeshFile.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t VertexStream::getDataOffset(unsigned long semantic) const
{
	if ((format & semantic) == 0)
	{
		return 0;
	}
	switch (semantic)
	{
	case POSITION:
		return 0;
	case NORMAL:
		return sizeof(Vector3);
	case TANGENT:
		return 2 * sizeof(Vector3);
	case TEXCOORD:
        {
            size_t offset = 0;
            if( format & POSITION )
            {
                offset += sizeof(Vector3);
            }
            if( format & NORMAL )
            {
                offset += sizeof(Vector3);
            }
            if( format & TANGENT )
            {
                offset += 2 * sizeof(Vector3);
            }
            return offset;
        }
	case TEXCOORD2:
        {
            size_t offset = 0;
            if( format & POSITION )
            {
                offset += sizeof(Vector3);
            }
            if( format & NORMAL )
            {
                offset += sizeof(Vector3);
            }
            if( format & TANGENT )
            {
                offset += 2 * sizeof(Vector3);
            }
            if( format & TEXCOORD )
            {
                offset += sizeof(Vector2);
            }
            return offset;
        }
	case COLOR:
        {
            size_t offset = 0;
            if( format & POSITION )
            {
                offset += sizeof(Vector3);
            }
            if( format & NORMAL )
            {
                offset += sizeof(Vector3);
            }
            if( format & TANGENT )
            {
                offset += 2 * sizeof(Vector3);
            }
            if( format & TEXCOORD )
            {
                offset += sizeof(Vector2);
            }
            if( format & TEXCOORD2 )
            {
                offset += sizeof(Vector2);
            }
            return offset;
        }
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Mesh::Mesh(const MeshFile* meshFile)
	: m_meshFile(meshFile)
	, m_boundingBox(AaBox::EMPTY)
	, m_type(0)
	, m_vertexCount(0)
	, m_lodTolerance(0.0f)
	, m_userData(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const Char* Mesh::getName() const
{
	if (m_meshFile != NULL)
	{
		return m_meshFile->getName().c_str();
	}
	return GT("");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const Char* Mesh::getProperty() const
{
	if (m_meshFile != NULL)
	{
		return m_meshFile->getProperty().c_str();
	}
	return GT("");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long Mesh::getVertexFormat() const
{
	if (m_meshFile == NULL)
	{
		return 0;
	}
	return m_meshFile->getVertexFormat();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Mesh::checkVertexFormat(unsigned long format) const
{
	if (m_meshFile == NULL)
	{
		return false;
	}
	return m_meshFile->checkVertexFormat(format);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t Mesh::getMaxVertexCount() const
{
	assert(m_meshFile != NULL);
	return m_meshFile->getVertexCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::setLodTolerance(float tolerance, IEventHandler* eventHandler)
{
	assert(m_meshFile != NULL);
	for (size_t i = 0; i < m_buffers.size(); ++i)
	{
		m_buffers[i].indices = &(m_meshFile->getBufferLodIndices(i, tolerance));
	}
	m_vertexCount = m_meshFile->getLodVertexCount(tolerance);
	m_lodTolerance = tolerance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::build()
{
	assert(m_meshFile != NULL);
	m_vertexCount = m_meshFile->getVertexCount();
	m_type = m_meshFile->getType();
	m_buffers.resize(m_meshFile->getBufferCount());
	for (size_t i = 0; i < m_buffers.size(); ++i)
	{
		MeshBuffer& buffer = m_buffers[i];
		buffer.indices = &(m_meshFile->getBufferLodIndices(i, 0.0f));
		buffer.maxIndexCount = buffer.indices->size();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Mesh::calculateBoundingBox()
{
	assert(isBuilt());

	if (getVertexCount() == 0)
	{
		return;
	}
	if (m_meshFile == NULL || !m_meshFile->checkVertexFormat(POSITION))
	{
		return;
	}
	VertexStream* stream = NULL;
	if ((m_staticStream.format & POSITION) != 0)
	{
		stream = &m_staticStream;
	}
	else if ((m_dynamicStream.format & POSITION) != 0)
	{
		stream = &m_dynamicStream;
	}
	if (stream == NULL)
	{
		return;
	}
	unsigned char* positionPtr = stream->buffer + MeshFile::getDataOffset(stream->format, POSITION);
	const Vector3* position = reinterpret_cast<const Vector3*>(positionPtr);
	m_boundingBox.reset(*position);
	size_t vertexCount = getBBVertexCount();
	for (size_t vertex = 1; vertex < vertexCount; ++vertex, positionPtr += stream->stride)
	{
		position = reinterpret_cast<const Vector3*>(positionPtr);
		m_boundingBox.addInternalPoint(*position);
	}
	if (checkType(MESH_TRANSFORM))
	{
		m_boundingBox = getTransform().transformBox(m_boundingBox);
	}
}

}
