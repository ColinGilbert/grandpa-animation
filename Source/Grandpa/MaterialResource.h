#ifndef __GRP_MATERIAL_RESOURCE_H__
#define __GRP_MATERIAL_RESOURCE_H__

#include "Resource.h"
#include "IMaterial.h"
#include "Property.h"

namespace slim
{
class XmlNode;
}

namespace grp
{

const Char* const TexCoordModeNames[] = 
{
	GT(""),
	GT("wrap"),
	GT("mirror"),
	GT("clamp"),
	GT("border")
};

const Char* const TriangleCullModeNames[] = 
{
	GT(""),
	GT("none"),
	GT("cw"),
	GT("ccw")
};

const Char* const ZBufferModeNames[] = 
{
	GT(""),
	GT("never"),
	GT("less"),
	GT("equal"),
	GT("lessequal"),
	GT("greater"),
	GT("notequal"),
	GT("greaterequal"),
	GT("always")
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class TextureResource
{
public:
	TextureResource()
		: m_texcoordMode(TEXCOORD_WRAP)
		, m_linearFilter(true)
		, m_mipmap(true)
	{}

public:
	STRING			m_filename;
	TexcoordMode	m_texcoordMode;
	bool			m_linearFilter;
	bool			m_mipmap;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class MaterialResource : public Resource
{
public:
	MaterialResource(bool managed);

	virtual ResourceType getResourceType() const;

	virtual bool importXml(const void* buffer, size_t size, void* param0, void* param1);
	void importXmlNode(slim::XmlNode* node);

private:
	void readTexture(slim::XmlNode* node);

public:
	Property					m_userProperty;
	STRING						m_type;
	TriangleCullMode			m_cullMode;
	ZBufferMode					m_zMode;
	Color32						m_color;
	VECTOR(TextureResource)		m_textures;
	bool						m_fog;
	bool						m_zEnable;
	bool						m_zWrite;
	bool						m_wireframe;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ResourceType MaterialResource::getResourceType() const
{
	return RES_TYPE_MATERIAL;
}

}

#endif
