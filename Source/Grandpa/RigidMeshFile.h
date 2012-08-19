#ifndef __GRP_RIGID_MESH_FIIE_H__
#define __GRP_RIGID_MESH_FIIE_H__

#include "MeshFile.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
class RigidMeshFile : public MeshFile
{
	friend class CExporter;

public:
	virtual unsigned long getStaticStreamFormat() const;
	virtual unsigned long getDynamicStreamFormat() const;

	virtual bool importFrom(std::istream& input);

	const STRING& getAttachedBoneName() const;

private:
	STRING	m_attachedBoneName;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline unsigned long RigidMeshFile::getStaticStreamFormat() const
{
	return m_vertexFormat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline unsigned long RigidMeshFile::getDynamicStreamFormat() const
{
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const STRING& RigidMeshFile::getAttachedBoneName() const
{
	return m_attachedBoneName;
}

}

#endif
