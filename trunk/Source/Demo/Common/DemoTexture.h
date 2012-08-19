#ifndef __DEMO_TEXTURE_H__
#define __DEMO_TEXTURE_H__

#include "IResource.h"
#include <string>

struct IDirect3DDevice9;
struct IDirect3DTexture9;

#define RES_TYPE_TEXTURE	grp::RES_TYPE_USER0

class DemoTexture : public grp::IResource
{
public:
	DemoTexture();
	virtual ~DemoTexture();

	virtual grp::ResourceType getResourceType() const;

	virtual const wchar_t* getResourceUrl() const;
	virtual const wchar_t* getFilePath() const;
	void setResourceUrl(const wchar_t* url);

	virtual grp::ResourceState getResourceState() const;
	void setResourceState(grp::ResourceState state);

	virtual bool allComplete() const;

	virtual void setPriority(float priority) const;
	virtual float getPriority() const;

	virtual void setUserData(const void* data);
	virtual const void* getUserData() const;

	virtual float getFreeDelay() const;

	void grab();
	bool drop();

	void build(IDirect3DDevice9* device);
	void destroy();

	IDirect3DTexture9* getD3dTexture();

private:
	IDirect3DTexture9*	m_d3dTexture;
	grp::ResourceState	m_state;
	std::wstring		m_url;
	int					m_referenceCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline grp::ResourceType DemoTexture::getResourceType() const
{
	return RES_TYPE_TEXTURE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const wchar_t* DemoTexture::getResourceUrl() const
{
	return m_url.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const wchar_t* DemoTexture::getFilePath() const
{
	return m_url.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoTexture::setResourceUrl(const wchar_t* url)
{
	m_url = url;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline grp::ResourceState DemoTexture::getResourceState() const
{
	return m_state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoTexture::setResourceState(grp::ResourceState state)
{
	m_state = state;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool DemoTexture::allComplete() const
{
	return (m_state == grp::RES_STATE_COMPLETE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoTexture::setPriority(float priority) const
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoTexture::getPriority() const
{
	return 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoTexture::setUserData(const void* data)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const void* DemoTexture::getUserData() const
{
	return NULL;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoTexture::getFreeDelay() const
{
	return 30.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoTexture::grab()
{
	++m_referenceCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool DemoTexture::drop()
{
	--m_referenceCount;
	return (m_referenceCount <= 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IDirect3DTexture9* DemoTexture::getD3dTexture()
{
	return m_d3dTexture;
}

#endif