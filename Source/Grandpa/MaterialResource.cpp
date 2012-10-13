#include "Precompiled.h"
#include "MaterialResource.h"
#include "SlimXml.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
MaterialResource::MaterialResource(bool managed)
	: Resource(managed)
	, m_cullMode(CULL_CCW)
	, m_zMode(Z_LESSEQUAL)
	, m_color(0)
	, m_fog(true)
	, m_zEnable(true)
	, m_zWrite(true)
	, m_wireframe(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool MaterialResource::importXml(const void* buffer, size_t size, void* param0, void* param1)
{
	slim::XmlDocument xmlFile;
	if (!xmlFile.loadFromMemory((const char*)buffer, size))
	{
		return false;
	}
	slim::XmlNode* node = xmlFile.findChild(GT("material"));
	if (node == NULL)
	{
		return false;
	}
	importXmlNode(node);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MaterialResource::importXmlNode(slim::XmlNode* node)
{
	assert(node != NULL);
	
	STRING temp;
	m_type = node->readAttribute<const Char*>(GT("type"), GT(""));
	m_cullMode = (TriangleCullMode)node->readAttributeAsEnum(GT("cullMode"), TriangleCullModeNames,
															 sizeof(TriangleCullModeNames)/sizeof(Char*));
	m_zMode = (ZBufferMode)node->readAttributeAsEnum(GT("zFunc"), ZBufferModeNames,
													 sizeof(ZBufferModeNames)/sizeof(Char*));
	m_color = node->readAttributeAsHex(GT("color"));
	m_fog = node->readAttribute<bool>(GT("fog"), false);
	m_zEnable = node->readAttribute<bool>(GT("zEnable"), true);
	m_zWrite = node->readAttribute<bool>(GT("zWrite"), true);
	m_wireframe = node->readAttribute<bool>(GT("wireframe"), false);

	m_textures.reserve(node->getChildCount(GT("texture")));
	m_userProperty.pairs.reserve(node->getChildCount(GT("property")));
	slim::NodeIterator nodeIter;
	for (slim::XmlNode* child = node->getFirstChild(nodeIter);
		child != NULL;
		child = node->getNextChild(nodeIter))
	{
		if (Strcmp(child->getName(), GT("texture")) == 0)
		{
			readTexture(child);
		}
		else if (Strcmp(child->getName(), GT("property")) == 0)
		{
			readPropertyFromNode(child, m_userProperty.pairs);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void MaterialResource::readTexture(slim::XmlNode* node)
{
	assert(node != NULL);

	m_textures.resize(m_textures.size() + 1);
	TextureResource& texture = m_textures.back();
	texture.m_filename = node->readAttribute<const Char*>(GT("filename"), GT(""));
	texture.m_linearFilter = node->readAttribute<bool>(GT("linearFilter"), false);
	texture.m_linearFilter = node->readAttribute<bool>(GT("mipmap"), false);
	texture.m_texcoordMode = (TexcoordMode)node->readAttributeAsEnum(GT("texAddress"), TexCoordModeNames,
																	 sizeof(TexCoordModeNames)/sizeof(Char*));
}

}
