#include "Precompiled.h"
#include "PartResource.h"
#include "MaterialResource.h"
#include "ResourceFactory.h"
#include "SlimXml.h"

namespace grp
{

extern ResourceFactory* g_resourceFactory;

///////////////////////////////////////////////////////////////////////////////////////////////////
PartResource::PartResource(bool managed)
	: Resource(managed)
	, m_meshResource(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
PartResource::~PartResource()
{
	SAFE_DROP(m_meshResource);
	for (VECTOR(MaterialResource*)::iterator iter = m_materialResources.begin();
		iter != m_materialResources.end();
		++iter)
	{
		(*iter)->drop();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PartResource::setPriority(float priority) const
{
	Resource::setPriority(priority);// * PRIORITY_PART);
	if (m_meshResource != NULL)
	{
		m_meshResource->setPriority(priority);// * PRIORITY_MESH);
	}
	for (VECTOR(MaterialResource*)::const_iterator iter = m_materialResources.begin();
		iter != m_materialResources.end();
		++iter)
	{
		(*iter)->setPriority(priority);// * PRIORITY_MATERIAL);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PartResource::allComplete() const
{
	if (!Resource::allComplete())
	{
		return false;
	}
	for (VECTOR(MaterialResource*)::const_iterator iter = m_materialResources.begin();
		iter != m_materialResources.end();
		++iter)
	{
		if (!(*iter)->allComplete())
		{
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool PartResource::importXml(const void* buffer, size_t size, void* param0, void* param1)
{
	slim::XmlDocument xmlFile;
	if (!xmlFile.loadFromMemory((const char*)buffer, size))
	{
		return false;
	}
	slim::XmlNode* node = xmlFile.findChild(GT("part"));
	if (node == NULL)
	{
		return false;
	}
	importXmlNode(node, param0, param1);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PartResource::importXmlNode(slim::XmlNode* node, void* param0, void* param1)
{
	assert(node != NULL);

	m_userProperty.pairs.reserve(node->getChildCount(GT("property")));
	m_materialResources.reserve(node->getChildCount(GT("material")));

	//for compatibility
	readMesh(node, param0, param1, GT("mesh"));

	slim::NodeIterator nodeIter;
	for (slim::XmlNode* child = node->getFirstChild(nodeIter);
		child != NULL;
		child = node->getNextChild(nodeIter))
	{
		if (Strcmp(child->getName(), GT("mesh")) == 0)
		{
			if (m_meshResource == NULL)
			{
				readMesh(child, param0, param1, GT("filename"));
			}
		}
		else if (Strcmp(child->getName(), GT("material")) == 0)
		{
			readMaterial(child, param0, param1);
		}
		else if (Strcmp(child->getName(), GT("property")) == 0)
		{
			readPropertyFromNode(child, m_userProperty.pairs);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PartResource::readMesh(slim::XmlNode* node, void* param0, void* param1, const slim::Char* meshAttrName)
{
	assert(node != NULL);

	slim::XmlAttribute* filenameAttr = node->findAttribute(meshAttrName);
	if (filenameAttr == NULL)
	{
		return;
	}
	ResourceType resType;
	slim::XmlAttribute* typeAttr = node->findAttribute(GT("type"));
	if (typeAttr != NULL && Strcmp(typeAttr->getString(), GT("skinned")) == 0)
	{
		resType = RES_TYPE_SKINNED_MESH;
	}
	else
	{
		resType = RES_TYPE_RIGID_MESH;
	}
	IResource* resource = grabChildResource(resType, filenameAttr->getString(), param0, param1);
	m_meshResource = static_cast<Resource*>(resource);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void PartResource::readMaterial(slim::XmlNode* node, void* param0, void* param1)
{
	assert(node != NULL);

	IResource* resource = NULL;
	slim::XmlAttribute* filenameAttr = node->findAttribute(GT("filename"));
	if (filenameAttr == NULL)
	{
		//embedded material data, create resource from factory
		assert(g_resourceFactory != NULL);
		resource = g_resourceFactory->createResource(getResourceUrl(), RES_TYPE_MATERIAL, param0, param1, false);
		MaterialResource* materialResource = static_cast<MaterialResource*>(resource);
		materialResource->importXmlNode(node);
		materialResource->setResourceState(RES_STATE_COMPLETE);
		materialResource->grab();
	}
	else
	{
		resource = grabChildResource(RES_TYPE_MATERIAL, filenameAttr->getString(), param0, param1);
	}
	m_materialResources.push_back(static_cast<MaterialResource*>(resource));
}

}
