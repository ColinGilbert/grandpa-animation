#ifndef __GRP_I_SKIN_H__
#define __GRP_I_SKIN_H__

namespace grp
{

const size_t MAX_VERTEX_INFLUENCE = 4;

///////////////////////////////////////////////////////////////////////////////////////////////////
struct VertexInfluence
{
	VertexInfluence()
		: weight(0.0f)
		, boneIndex(0xffffffff)
	{
	}

	unsigned long	boneIndex;	//index, not bone id in skeleton
	float			weight;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class ISkin
{
public:
	virtual size_t getBoneCount() const = 0;

	virtual const Matrix* getBoneMatrices() const = 0;

	virtual size_t getSkinVertexCount() const = 0;

	virtual const VertexInfluence* getVertexInfluences(size_t vertexIndex) const = 0;

	virtual void setGpuSkinning(bool enable) = 0;
	virtual bool isGpuSkinning() const = 0;

	virtual void setUserData(void* data) = 0;
	virtual void* getUserData() const = 0;

protected:
	virtual ~ISkin(){}
};

}

#endif
