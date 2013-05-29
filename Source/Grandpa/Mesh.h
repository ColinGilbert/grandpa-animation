#ifndef __GRP_MESH_H__
#define __GRP_MESH_H__

#include "IMesh.h"
#include "ResourceInstance.h"

namespace grp
{

class Resource;
class MeshFile;
class MeshBuffer;

///////////////////////////////////////////////////////////////////////////////////////////////////
class VertexStream : public IVertexStream
{
public:
	VertexStream()
		: stride(0)
		, format(0)
		, buffer(NULL)
		, userData(0)
	{}

	virtual size_t getStride() const;
	virtual unsigned long getFormat() const;
	virtual size_t getDataOffset(unsigned long semantic) const;
	virtual const unsigned char* getBuffer() const;

	virtual void setUserData(void* data);
	virtual void* getUserData() const;

public:
	size_t			stride;
	unsigned long	format;
	unsigned char*	buffer;
	void*			userData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class Mesh : public IMesh, public ResourceInstance
{
public:
	Mesh(const MeshFile* meshFile);

	virtual const Char* getName() const;

	virtual const Char* getProperty() const;

	virtual bool checkType(unsigned long type) const;

	bool checkVertexFormat(unsigned long format) const;

	virtual unsigned long getVertexFormat() const;

	virtual IVertexStream* getStaticVertexStream() const;
	virtual IVertexStream* getDynamicVertexStream() const;

	virtual size_t getMaxVertexCount() const;
	virtual size_t getVertexCount() const;

	virtual size_t getMeshBufferCount() const;
	virtual IMeshBuffer* getMeshBuffer(size_t index = 0);

	virtual const AaBox& getBoundingBox() const;

	virtual ISkin* getSkin();

	virtual void setUserData(void* data);
	virtual void* getUserData() const;

	virtual const Resource* getMeshResource() const = 0;

	void setLodTolerance(float tolerance, IEventHandler* eventHandler);

	virtual void build();

	virtual void calculateBoundingBox();

	virtual size_t getBBVertexCount() const;

protected:
	const MeshFile*		m_meshFile;
	unsigned long		m_type;
	VertexStream		m_staticStream;
	VertexStream		m_dynamicStream;
	VECTOR(MeshBuffer)	m_buffers;
	AaBox				m_boundingBox;
	float				m_lodTolerance;
	size_t				m_vertexCount;
	void*				m_userData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class MeshBuffer : public IMeshBuffer
{
public:
	MeshBuffer()
		: maxIndexCount(0)
		, userData(0)
	{}

	virtual size_t getMaxIndexCount() const;
	virtual size_t getIndexCount() const;
	virtual const Index32* getIndices() const;

	virtual void setUserData(void* data);
	virtual void* getUserData() const;

public:
	size_t					maxIndexCount;
	const VECTOR(Index32)*	indices;
	void*					userData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Mesh::checkType(unsigned long type) const
{
	return ((m_type & type) != 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IVertexStream* Mesh::getStaticVertexStream() const
{
	return const_cast<VertexStream*>(&m_staticStream);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IVertexStream* Mesh::getDynamicVertexStream() const
{
	return const_cast<VertexStream*>(&m_dynamicStream);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t Mesh::getVertexCount() const
{
	return m_vertexCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t Mesh::getMeshBufferCount() const
{
	return m_buffers.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IMeshBuffer* Mesh::getMeshBuffer(size_t index)
{
	if( index >= 0 && index < m_buffers.size())
    {
        return &m_buffers[index];
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const AaBox& Mesh::getBoundingBox() const
{
	return m_boundingBox;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ISkin* Mesh::getSkin()
{
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Mesh::setUserData(void* data)
{
	m_userData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void* Mesh::getUserData() const
{
	return m_userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t Mesh::getBBVertexCount() const
{
	return getVertexCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t MeshBuffer::getMaxIndexCount() const
{
	return maxIndexCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t MeshBuffer::getIndexCount() const
{
	return indices->size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Index32* MeshBuffer::getIndices() const
{
	return &((*indices)[0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void MeshBuffer::setUserData(void* data)
{
	userData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void* MeshBuffer::getUserData() const
{
	return userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t VertexStream::getStride() const
{
	return stride;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline unsigned long VertexStream::getFormat() const
{
	return format;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const unsigned char* VertexStream::getBuffer() const
{
	return buffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void VertexStream::setUserData(void* data)
{
	userData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void* VertexStream::getUserData() const
{
	return userData;
}

}

#endif
