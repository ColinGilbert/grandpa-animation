#ifndef __GRP_MATERIAL_H__
#define __GRP_MATERIAL_H__

#include "IMaterial.h"
#include "MaterialResource.h"
#include "ResourceInstance.h"

namespace grp
{

class Texture : public ITexture
{
public:
	Texture();

	void setResource(TextureResource* resource);

	virtual const Char* filename() const;
	virtual TexcoordMode texcoordMode() const;
	virtual bool linearFilter() const;
	virtual bool mipmap() const;

	virtual void setUserData(void* data);
	virtual void* getUserData() const;

private:
	TextureResource*	m_resource;
	void*				m_userData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class Material : public IMaterial, public ResourceInstance
{
public:
	Material();
	~Material();

	void setResource(MaterialResource* resource);
    virtual const IResource* getResource() const;

	virtual void build();
    virtual bool isBuilt() const;

	const MaterialResource* getMaterialResource() const;

	virtual const Char* type() const;
	virtual TriangleCullMode cullMode() const;
	virtual ZBufferMode	zMode() const;
	virtual Color32 color() const;
	virtual bool fog() const;
	virtual bool zEnable() const;
	virtual bool zWrite() const;
	virtual bool wireframe() const;

	virtual size_t getTextureCount() const;
	virtual ITexture* getTexture(size_t index);

	virtual IProperty* getProperty() const;

	virtual void setUserData(void* data);
	virtual void* getUserData() const;

private:
	MaterialResource*	m_resource;
	VECTOR(Texture)		m_textures;
	void*				m_userData;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Texture::Texture()
	: m_resource(NULL)
	, m_userData(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Texture::setResource(TextureResource* resource)
{
	assert(resource != NULL);
	m_resource = resource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Char* Texture::filename() const
{
	assert(m_resource != NULL);
	return m_resource->m_filename.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline TexcoordMode Texture::texcoordMode() const
{
	assert(m_resource != NULL);
	return m_resource->m_texcoordMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Texture::linearFilter() const
{
	assert(m_resource != NULL);
	return m_resource->m_linearFilter;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Texture::mipmap() const
{
	assert(m_resource != NULL);
	return m_resource->m_mipmap;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Texture::setUserData(void* data)
{
	m_userData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void* Texture::getUserData() const
{
	return m_userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Material::Material()
	: m_resource(NULL)
	, m_userData(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Material::~Material()
{
	if (m_resource != NULL)
	{
		m_resource->drop();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Material::setResource(MaterialResource* resource)
{
	assert(resource != NULL);
	m_resource = resource;
	resource->grab();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Material::build()
{
	assert(m_resource->getResourceState() == RES_STATE_COMPLETE);
	size_t textureCount = m_resource->m_textures.size();
	m_textures.resize(textureCount);
	for (size_t i = 0; i < textureCount; ++i)
	{
		m_textures[i].setResource(&m_resource->m_textures[i]);
	}
	setBuilt();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Material::isBuilt() const
{
    return ResourceInstance::isBuilt();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const MaterialResource* Material::getMaterialResource() const
{
	return m_resource;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline const IResource* Material::getResource() const
{
    return m_resource;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Char* Material::type() const
{
	if (!isBuilt())
	{
		return GT("");
	}
	assert(m_resource != NULL);
	return m_resource->m_type.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline TriangleCullMode Material::cullMode() const
{
	if (!isBuilt())
	{
		return CULL_NONE;
	}
	assert(m_resource != NULL);
	return m_resource->m_cullMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ZBufferMode Material::zMode() const
{
	if (!isBuilt())
	{
		return Z_LESSEQUAL;
	}
	assert(m_resource != NULL);
	return m_resource->m_zMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Color32 Material::color() const
{
	if (!isBuilt())
	{
		return 0;
	}
	assert(m_resource != NULL);
	return m_resource->m_color;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Material::fog() const
{
	if (!isBuilt())
	{
		return false;
	}
	assert(m_resource != NULL);
	return m_resource->m_fog;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Material::zEnable() const
{
	if (!isBuilt())
	{
		return true;
	}
	assert(m_resource != NULL);
	return m_resource->m_zEnable;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Material::zWrite() const
{
	if (!isBuilt())
	{
		return true;
	}
	assert(m_resource != NULL);
	return m_resource->m_zWrite;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Material::wireframe() const
{
	if (!isBuilt())
	{
		return false;
	}
	assert(m_resource != NULL);
	return m_resource->m_wireframe;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t Material::getTextureCount() const
{
	if (!isBuilt())
	{
		return 0;
	}
	return m_textures.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ITexture* Material::getTexture(size_t index)
{
	if (!isBuilt())
	{
		return NULL;
	}
	if (index >= m_textures.size())
	{
		return NULL;
	}
	return const_cast<Texture*>(&m_textures[index]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IProperty* Material::getProperty() const
{
	if (!isBuilt())
	{
		return NULL;
	}
	assert(m_resource != NULL);
	return const_cast<Property*>(&(m_resource->m_userProperty));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Material::setUserData(void* data)
{
	m_userData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void* Material::getUserData() const
{
	return m_userData;
}

}

#endif
