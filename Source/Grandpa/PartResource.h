#ifndef __GRP_PART_RESOURCE_H__
#define __GRP_PART_RESOURCE_H__

#include "Resource.h"
#include "Property.h"

namespace slim
{
class XmlNode;
}

namespace grp
{
class MaterialResource;

class PartResource : public Resource
{
public:
	PartResource(bool managed);
	virtual ~PartResource();

	virtual ResourceType getResourceType() const;

	virtual bool allComplete() const;

	virtual void setPriority(float priority) const;

	virtual bool importXml(const void* buffer, size_t size, void* param0, void* param1);
	void importXmlNode(slim::XmlNode* node, void* param0, void* param1);

	const Resource* getMeshResource() const;

	const VECTOR(MaterialResource*)& getMaterialResources() const;

	IProperty* getProperty() const;

private:
	void readMesh(slim::XmlNode* node, void* param0, void* param1, const slim::Char* meshAttrName);
	void readMaterial(slim::XmlNode* node, void* param0, void* param1);

private:
	Property			m_userProperty;
	const Resource*		m_meshResource;
	VECTOR(MaterialResource*)	m_materialResources;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ResourceType PartResource::getResourceType() const
{
	return RES_TYPE_PART;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Resource* PartResource::getMeshResource() const
{
	return m_meshResource;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(MaterialResource*)& PartResource::getMaterialResources() const
{
	return m_materialResources;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IProperty* PartResource::getProperty() const
{
	return const_cast<Property*>(&m_userProperty);
}

}

#endif
