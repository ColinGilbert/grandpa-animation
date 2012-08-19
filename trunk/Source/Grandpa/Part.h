#ifndef __GRP_PART_H__
#define __GRP_PART_H__

#include "IPart.h"
#include "ResourceInstance.h"
#include "Material.h"

namespace grp
{
class PartResource;
class Mesh;

class Part : public IPart, public ResourceInstance
{
public:
	Part(const PartResource* resource);
	virtual ~Part();

	virtual IResource* getResource() const;

	virtual void setVisible(bool visible);
	virtual bool isVisible() const;

	virtual bool isSkinnedPart() const;

	virtual IMesh* getMesh() const;

	virtual size_t getMaterialCount() const;

	virtual IMaterial* getMaterial(size_t index) const;

	virtual IProperty* getProperty() const;

	virtual void setUserData(void* data);
	virtual void* getUserData() const;

	const PartResource* getPartResource() const;

	void update(IModel* model, IEventHandler* eventHandler);

	void build(IModel* model, IEventHandler* eventHandler);

	void buildMesh(float lodTolerance, IEventHandler* eventHandler);
	virtual bool isMeshBuilt() const;

private:
	const PartResource*	m_resource;
	VECTOR(Material)	m_materials;
	Mesh*				m_mesh;
	void*				m_userData;
	bool				m_visible;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IResource* Part::getResource() const
{
	return const_cast<PartResource*>(m_resource);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Part::setVisible(bool visible)
{
	m_visible = visible;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Part::isVisible() const
{
	return m_visible;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t Part::getMaterialCount() const
{
	return m_materials.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline IMaterial* Part::getMaterial(size_t index) const
{
	return const_cast<Material*>(&m_materials[index]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void Part::setUserData(void* data)
{
	m_userData = data;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void* Part::getUserData() const
{
	return m_userData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const PartResource* Part::getPartResource() const
{
	return m_resource;
}

}

#endif
