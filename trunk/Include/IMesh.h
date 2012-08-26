#ifndef __GRP_IMESH_H__
#define __GRP_IMESH_H__

namespace grp
{

const unsigned long MESH_RIGID		= 0x00000001L;	//rigid(or skinned)
const unsigned long MESH_LOD		= 0x00000002L;	//has mesh LOD
const unsigned long MESH_TRANSFORM	= 0x00000004L;	//has transform
const unsigned long MESH_STANDALONE	= 0x00000008L;	//is standalone mesh

const unsigned long POSITION	= 0x00000100L;
const unsigned long NORMAL		= 0x00000200L;
const unsigned long TANGENT		= 0x00000400L;
const unsigned long TEXCOORD	= 0x00000800L;
const unsigned long TEXCOORD2	= 0x00001000L;
const unsigned long COLOR		= 0x00002000L;
const unsigned long BLENDINDICES= 0x00004000L;
const unsigned long BLENDWEIGHT	= 0X00008000L;

class IMeshBuffer;
class ISkin;
class IVertexStream;

////////////////////////////////////////////////////////////////////////////////////////////////////
enum MeshUpdateMode
{
	UPDATE_DEFAULT = 0,
	UPDATE_NO_TANGENT,
	UPDATE_POS_ONLY
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class IMesh
{
public:
	virtual const Char* getName() const = 0;

	virtual const Char* getProperty() const = 0;

	virtual unsigned long getVertexFormat() const = 0;

	virtual size_t getMaxVertexCount() const = 0;
	virtual size_t getVertexCount() const = 0;

	virtual IVertexStream* getStaticVertexStream() const = 0;
	virtual IVertexStream* getDynamicVertexStream() const = 0;

	virtual size_t getMeshBufferCount() const = 0;
	virtual IMeshBuffer* getMeshBuffer(size_t index = 0) = 0;

	//transform in model space
	virtual const grp::Matrix& getTransform() const = 0;

	virtual const AaBox& getBoundingBox() const = 0;

	virtual ISkin* getSkin() = 0;

	virtual void setUserData(void* data) = 0;
	virtual void* getUserData() const = 0;

	virtual void setUpdateMode(MeshUpdateMode mode){}
	virtual MeshUpdateMode getUpdateMode() const {return UPDATE_DEFAULT;}

protected:
	virtual ~IMesh(){}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class IMeshBuffer
{
public:
	virtual size_t getMaxIndexCount() const = 0;
	virtual size_t getIndexCount() const = 0;
	virtual const Index32* getIndices() const = 0;

	//save index buffer pointer here :)
	virtual void setUserData(void* data) = 0;
	virtual void* getUserData() const = 0;

protected:
	virtual ~IMeshBuffer(){}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class IVertexStream
{
public:
	virtual size_t getStride() const = 0;
	virtual unsigned long getFormat() const = 0;
	virtual size_t getDataOffset(unsigned long semantic) const = 0;
	virtual const unsigned char* getBuffer() const = 0;

	virtual void setUserData(void* data) = 0;
	virtual void* getUserData() const = 0;	
};

}

#endif
