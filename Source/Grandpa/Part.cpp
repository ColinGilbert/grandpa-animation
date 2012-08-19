#include "Precompiled.h"
#include "Part.h"
#include "PartResource.h"
#include "ContentResource.h"
#include "MaterialResource.h"
#include "SkinnedMesh.h"
#include "RigidMesh.h"
#include <vector>
#include "Performance.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
Part::Part(const PartResource* resource)
	: m_resource(resource)
	, m_mesh(NULL)
	, m_visible(true)
	, m_userData(0)
{
	assert(resource != NULL);
	resource->grab();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Part::~Part()
{
	if (m_mesh != NULL)
	{
		GRP_DELETE(m_mesh);
	}
	assert(m_resource != NULL);
	SAFE_DROP(m_resource);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IProperty* Part::getProperty() const
{
	if (m_resource != NULL)
	{
		return m_resource->getProperty();
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Part::update(IModel* model, IEventHandler* eventHandler)
{
	if (!m_visible)
	{
		return;
	}
	for (size_t i = 0; i < m_materials.size(); ++i)
	{
		Material& material = m_materials[i];
		const MaterialResource* materialResource = material.getMaterialResource();
		assert(materialResource != NULL);
		if (!material.isBuilt() && materialResource->getResourceState() == RES_STATE_COMPLETE)
		{
			material.build();
			if (eventHandler != NULL)
			{
				eventHandler->onMaterialBuilt(model, this, i);
			}
		}
	}

	if (m_mesh == NULL || !m_mesh->isBuilt())
	{
		return;
	}
	if (isSkinnedPart())
	{
		static_cast<SkinnedMesh*>(m_mesh)->update();
	}
	else
	{
		static_cast<RigidMesh*>(m_mesh)->update();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IMesh* Part::getMesh() const
{
	return m_mesh;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Part::isSkinnedPart() const
{
	if (m_mesh == NULL)
	{
		return false;
	}
	return (m_mesh->getMeshResource()->getResourceType() == RES_TYPE_SKINNED_MESH);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Part::build(IModel* model, IEventHandler* eventHandler)
{
	PERF_NODE_FUNC();

	assert(m_resource->getResourceState() == RES_STATE_COMPLETE);
	const Resource* meshResource = m_resource->getMeshResource();
	assert(meshResource != NULL);
	if (meshResource->getResourceState() == RES_STATE_BROKEN)
	{
		return;
	}
	assert(m_mesh == NULL);
	if (meshResource->getResourceType() == RES_TYPE_RIGID_MESH)
	{
		m_mesh = GRP_NEW RigidMesh(static_cast<const RigidMeshResource*>(meshResource));
	}
	else
	{
		m_mesh = GRP_NEW SkinnedMesh(static_cast<const SkinnedMeshResource*>(meshResource));
	}

	size_t materialCount = m_resource->getMaterialResources().size();
	m_materials.resize(materialCount);
	for (size_t i = 0; i < materialCount; ++i)
	{
		MaterialResource* materialResource = m_resource->getMaterialResources()[i];
		assert(materialResource != NULL);
		m_materials[i].setResource(materialResource);
		if (materialResource->getResourceState() == RES_STATE_COMPLETE)
		{
			m_materials[i].build();
			if (eventHandler != NULL)
			{
				eventHandler->onMaterialBuilt(model, this, i);
			}
		}
	}

	setBuilt();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Part::buildMesh(float lodTolerance, IEventHandler* eventHandler)
{
	assert(m_mesh != NULL);
	assert(!m_mesh->isBuilt());
	assert(m_mesh->getMeshResource()->getResourceState() == RES_STATE_COMPLETE);
	m_mesh->build();
	m_mesh->setLodTolerance(lodTolerance, eventHandler);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool Part::isMeshBuilt() const
{
	return (m_mesh != NULL && m_mesh->isBuilt());
}

}
