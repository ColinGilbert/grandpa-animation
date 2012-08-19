#ifndef __GRP_RESOURCE_INSTANCE_H__
#define __GRP_RESOURCE_INSTANCE_H__

namespace grp
{

class ResourceInstance
{
public:
	ResourceInstance()
		: m_isBuilt(false)
	{}

	bool isBuilt() const;

	void setBuilt();

private:
	bool	m_isBuilt;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool ResourceInstance::isBuilt() const
{
	return m_isBuilt;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void ResourceInstance::setBuilt()
{
	m_isBuilt = true;
}

}

#endif
