#ifndef __GRP_RESOURCE_H__
#define __GRP_RESOURCE_H__

#include "ReferenceCounted.h"
#include "IResource.h"
#include "IFileLoader.h"
#include "SlimXml.h"
#include <istream>

namespace slim
{
class XmlNode;
}

namespace grp
{

//static const float PRIORITY_MODEL = 10.0f;
//static const float PRIORITY_PART = 9.0f;
//static const float PRIORITY_MATERIAL = 8.0f;
//static const float PRIORITY_SKELETON = 7.5f;
//static const float PRIORITY_MESH = 7.0f;
//static const float PRIORITY_ANIMATION = 6.0f;

struct PropertyPair;

class Resource : public ReferenceCounted, public IResource, public IFileCallback
{
public:
	Resource(bool managed);
	virtual ~Resource();

	virtual void free() const;

	virtual const Char* getResourceUrl() const;
	virtual const Char* getFilePath() const;
	void setResourceUrl(const Char* url);

	virtual ResourceState getResourceState() const;
	void setResourceState(ResourceState state);

	virtual bool allComplete() const;

	virtual void setPriority(float priority) const;
	virtual float getPriority() const;

	virtual void setUserData(const void* data);
	virtual const void* getUserData() const;

	virtual float getFreeDelay() const;

	virtual bool importXml(const void* buffer, size_t size, void* param0, void* param1);

	virtual bool importBinary(std::istream& input, void* param0, void* param1);

	//from IFileCallback
	virtual bool wantBuffer() const;
	virtual void onFileComplete(const Char* path, void* param0, void* param1){}
	virtual void onFileComplete(const void* buffer, unsigned long size, void* param0, void* param1);
	virtual void onFileNotFound(void* param0, void* param1);

	//for speed
	const STRING& getResourceUrlStr() const;

	IResource* grabChildResource(ResourceType type, const STRING& url, void* param0, void* param1) const;

protected:
	void readPropertyFromNode(slim::XmlNode* node, VECTOR(PropertyPair)& pairs);

private:
	STRING			m_url;
	volatile ResourceState	m_state;
	mutable float	m_priority;
	const void*		m_userData;
	bool			m_managed;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Char* Resource::getResourceUrl() const
{
	return m_url.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Char* Resource::getFilePath() const
{
	return m_url.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Resource::setResourceUrl(const Char* url)
{
	m_url = url;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ResourceState Resource::getResourceState() const
{
	return m_state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Resource::setResourceState(ResourceState state)
{
	m_state = state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Resource::wantBuffer() const
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Resource::allComplete() const
{
	return (m_state == RES_STATE_COMPLETE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Resource::setPriority(float priority) const
{
	m_priority = priority;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float Resource::getPriority() const
{
	return m_priority;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Resource::setUserData(const void* data)
{
	m_userData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const void* Resource::getUserData() const
{
	return m_userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float Resource::getFreeDelay() const
{
	return 30.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Resource::importXml(const void* buffer, size_t size, void* param0, void* param1)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Resource::importBinary(std::istream& input, void* param0, void* param1)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const STRING& Resource::getResourceUrlStr() const
{
	return m_url;
}

}

#endif
