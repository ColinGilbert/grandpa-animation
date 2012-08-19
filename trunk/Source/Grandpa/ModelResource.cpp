#include "Precompiled.h"
#include "ModelResource.h"
#include "PartResource.h"
#include "ContentResource.h"
#include "SlimXml.h"
#include "ResourceFactory.h"
#include "Performance.h"

namespace grp
{

extern ResourceFactory* g_resourceFactory;

///////////////////////////////////////////////////////////////////////////////////////////////////
ModelResource::ModelResource(bool managed)
	: Resource(managed)
	, m_skeletonResource(NULL)
	, m_userParam0(NULL)
	, m_userParam1(NULL)
	, m_allComplete(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ModelResource::~ModelResource()
{
	SAFE_DROP(m_skeletonResource);

	for (VECTOR(PartInfo)::iterator iter = m_partInfo.begin();
		iter != m_partInfo.end();
		++iter)
	{
		SAFE_DROP(iter->resource);
	}
	for (MAP(STRING, AnimationInfo)::iterator iter = m_animationInfo.begin();
		iter != m_animationInfo.end();
		++iter)
	{
		SAFE_DROP(iter->second.resource);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelResource::setPriority(float priority) const
{
	Resource::setPriority(priority);// * PRIORITY_MODEL);
	if (m_skeletonResource != NULL)
	{
		m_skeletonResource->setPriority(priority);// * PRIORITY_SKELETON);
	}
	for (VECTOR(PartInfo)::const_iterator iter = m_partInfo.begin();
		iter != m_partInfo.end();
		++iter)
	{
		if (iter->resource != NULL)
		{
			iter->resource->setPriority(priority);
		}
	}
	for (MAP(STRING, AnimationInfo)::const_iterator iter = m_animationInfo.begin();
		iter != m_animationInfo.end();
		++iter)
	{
		if (iter->second.resource != NULL)
		{
			iter->second.resource->setPriority(priority);// * PRIORITY_ANIMATION);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ModelResource::importXml(const void* buffer, size_t size, void* param0, void* param1)
{
	//PERF_NODE_FUNC();

	slim::XmlDocument xmlFile;
	if (!xmlFile.loadFromMemory((const char*)buffer, size))
	{
		return false;
	}
	slim::XmlNode* node = xmlFile.findChild(GT("model"));
	if (node == NULL)
	{
		return false;
	}
	importXmlNode(node, param0, param1);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelResource::importXmlNode(slim::XmlNode* node, void* param0, void* param1)
{
	assert(node != NULL);
	
	m_userParam0 = param0;
	m_userParam1 = param1;

	slim::XmlAttribute* skeletonAttr = node->findAttribute(GT("skeleton"));
	if (skeletonAttr != NULL)
	{
		IResource* resource = grabChildResource(RES_TYPE_SKELETON, skeletonAttr->getString(), param0, param1);
		if (resource != NULL)
		{
			m_skeletonResource = static_cast<const SkeletonResource*>(resource);
		}
	}

	m_partInfo.reserve(node->getChildCount(GT("part")));
	m_userProperty.pairs.reserve(node->getChildCount(GT("property")));
	slim::NodeIterator nodeIter;
	for (slim::XmlNode* child = node->getFirstChild(nodeIter);
		child != NULL;
		child = node->getNextChild(nodeIter))
	{
		if (Strcmp(child->getName(), GT("part")) == 0)
		{
			readPartInfo(child);
		}
		else if (Strcmp(child->getName(), GT("animation")) == 0)
		{
			readAnimationInfo(child);
		}
		else if (Strcmp(child->getName(), GT("property")) == 0)
		{
			readPropertyFromNode(child, m_userProperty.pairs);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
const AnimationInfo* ModelResource::getAnimationInfo(const STRING& slot) const
{
	MAP(STRING, AnimationInfo)::const_iterator found = m_animationInfo.find(slot);
	if (found == m_animationInfo.end())
	{
		return NULL;
	}
	const AnimationInfo& info = found->second;
	if (info.resource == NULL)
	{
		//sorry for const_cast
		IResource* resource = grabChildResource(RES_TYPE_ANIMATION, info.filename, m_userParam0, m_userParam1);
		const_cast<AnimationInfo&>(info).resource = static_cast<const AnimationResource*>(resource);
	}
	return &info;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelResource::readPartInfo(slim::XmlNode* node)
{
	assert(node != NULL);

	slim::XmlAttribute* slotAttr = node->findAttribute(GT("slot"));
	if (slotAttr == NULL)
	{
		return;
	}
	m_partInfo.resize(m_partInfo.size() + 1);
	PartInfo& info = m_partInfo.back();
	info.slot = slotAttr->getString();

	slim::XmlAttribute* filenameAttr = node->findAttribute(GT("filename"));
	IResource* resource = NULL;
	if (filenameAttr == NULL)
	{
		//embedded part data, create resource from factory
		assert(g_resourceFactory != NULL);
		resource = g_resourceFactory->createResource(getResourceUrl(), RES_TYPE_PART, m_userParam0, m_userParam1, false);
		PartResource* partResource = static_cast<PartResource*>(resource);
		partResource->importXmlNode(node, m_userParam0, m_userParam1);
		partResource->setResourceState(RES_STATE_COMPLETE);
		partResource->grab();
		info.resource = partResource;
	}
	else
	{
		resource = grabChildResource(RES_TYPE_PART, filenameAttr->getString(), m_userParam0, m_userParam1);
		info.resource = static_cast<const PartResource*>(resource);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelResource::readAnimationInfo(slim::XmlNode* node)
{
	assert(node != NULL);

	slim::XmlAttribute* slotAttr = node->findAttribute(GT("slot"));
	slim::XmlAttribute* filenameAttr = node->findAttribute(GT("filename"));
	if (slotAttr == NULL || filenameAttr == NULL)
	{
		return;
	}
	AnimationInfo& info = m_animationInfo[slotAttr->getString()];
	info.filename = filenameAttr->getString();
	info.slot = slotAttr->getString();
	info.resource = NULL;
	info.startTime = node->readAttributeAsFloat(GT("start"));
	info.endTime = node->readAttributeAsFloat(GT("end"), 999.0f);
	if (node->readAttributeAsBool(GT("preload")))
	{
		IResource* resource = grabChildResource(RES_TYPE_ANIMATION, info.filename, m_userParam0, m_userParam1);
		info.resource = static_cast<const AnimationResource*>(resource);
	}
	info.events.reserve(node->getChildCount(GT("event")));
	slim::NodeIterator nodeIter;
	for (slim::XmlNode* child = node->getFirstChild(nodeIter);
		child != NULL;
		child = node->getNextChild(nodeIter))
	{
		if (Strcmp(child->getName(), GT("event")) == 0)
		{
			readAnimationEvent(child, info);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ModelResource::readAnimationEvent(slim::XmlNode* node, AnimationInfo& animationInfo)
{
	assert(node != NULL);

	slim::XmlAttribute* nameAttr = node->findAttribute(GT("name"));
	slim::XmlAttribute* typeAttr = node->findAttribute(GT("type"));
	if (nameAttr == NULL || typeAttr == NULL)
	{
		return;
	}
	animationInfo.events.resize(animationInfo.events.size() + 1);
	AnimationEvent& animationEvent = animationInfo.events.back();
	animationEvent.name = nameAttr->getString();
	if (Strcmp(typeAttr->getString(), GT("start")) == 0)
	{
		animationEvent.type = ANIMATION_EVENT_START;
		animationEvent.time = 0.0f;
	}
	else if (Strcmp(typeAttr->getString(), GT("end")) == 0)
	{
		animationEvent.type = ANIMATION_EVENT_END;
		animationEvent.time = 0.0f;
	}
	else
	{
		animationEvent.type = ANIMATION_EVENT_TIME;
		animationEvent.time = node->readAttributeAsFloat(GT("time"));
	}

	//params
	animationEvent.property.pairs.reserve(node->getChildCount(GT("param")));

	slim::NodeIterator nodeIter;
	for (slim::XmlNode* child = node->getFirstChild(nodeIter);
		child != NULL;
		child = node->getNextChild(nodeIter))
	{
		if (Strcmp(child->getName(), GT("param")) == 0)
		{
			readPropertyFromNode(child, animationEvent.property.pairs);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ModelResource::updateCompleteState() const
{
	if (!Resource::allComplete())
	{
		return false;
	}
	if (m_skeletonResource != NULL && !m_skeletonResource->allComplete())
	{
		return false;
	}
	for (VECTOR(PartInfo)::const_iterator iter = m_partInfo.begin();
		iter != m_partInfo.end();
		++iter)
	{
		if (!iter->resource->allComplete())
		{
			return false;
		}
	}
	for (MAP(STRING, AnimationInfo)::const_iterator iter = m_animationInfo.begin();
		iter != m_animationInfo.end();
		++iter)
	{
		//animation can be loaded when playing
		if (iter->second.resource != NULL && !iter->second.resource->allComplete())
		{
			return false;
		}
	}
	return true;
}

}
