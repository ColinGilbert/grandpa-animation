#include "Precompiled.h"
#include "Resource.h"
#include "IResourceManager.h"
#include "PathUtil.h"
#include "SlimXml.h"
#include <strstream>

namespace grp
{

extern IResourceManager* g_resourceManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
Resource::Resource(bool managed)
	: m_state(RES_STATE_LOADING)
	, m_priority(0.0f)
	, m_userData(NULL)
	, m_managed(managed)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Resource::~Resource()
{
	WRITE_LOG_HINT(INFO, GT("Resource destroyed:"), m_url.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Resource::free() const
{
	if (m_managed)
	{
		assert(g_resourceManager != NULL);
		g_resourceManager->freeResource(const_cast<Resource*>(this));
	}
	else
	{
		ReferenceCounted::free();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Resource::onFileComplete(const void* buffer, unsigned long size, void* param0, void* param1)
{
	assert(buffer != NULL);
	bool succeeded = false;
	if (!importXml(buffer, size, param0, param1))
	{
		std::istrstream ss((char*)buffer, size);
		if (!importBinary(ss, param0, param1))
		{
			setResourceState(RES_STATE_BROKEN);
			WRITE_LOG_HINT(ERROR, GT("Failed to load resource:"), getResourceUrl());
			return;
		}
	}
	setResourceState(RES_STATE_COMPLETE);
	WRITE_LOG_HINT(INFO, GT("Resource loaded:"), getResourceUrl());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Resource::onFileNotFound(void* param0, void* param1)
{
	setResourceState(RES_STATE_BROKEN);
	WRITE_LOG_HINT(ERROR, GT("Failed to open file:"), getResourceUrl());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IResource* Resource::grabChildResource(ResourceType type, const STRING& url, void* param0, void* param1) const
{
	assert(g_resourceManager != NULL);
	IResource* resource;
	if (grp::containFolder(url))
	{
		resource = g_resourceManager->getResource(url.c_str(), type, param0, param1);
	}
	else
	{
		STRING fullUrl;
		grp::getUrlBase(m_url, fullUrl);
		fullUrl += url;
		resource = g_resourceManager->getResource(fullUrl.c_str(), type, param0, param1);
	}
	static_cast<Resource*>(resource)->grab();
	return resource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Resource::readPropertyFromNode(slim::XmlNode* node, VECTOR(PropertyPair)& pairs)
{
	assert(node != NULL);

	slim::AttributeIterator iter;
	for (slim::XmlAttribute* attribute = node->getFirstAttribute(iter);
		attribute != NULL;
		attribute = node->getNextAttribute(iter))
	{
		pairs.resize(pairs.size() + 1);
		PropertyPair& pair = pairs.back();
		pair.name = attribute->getName();
		pair.value = attribute->getString();
	}
}

}
