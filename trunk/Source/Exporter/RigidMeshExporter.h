#ifndef __GRP_RIGID_MESH_EXPORTER_H__
#define __GRP_RIGID_MESH_EXPORTER_H__

#include "MeshExporter.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
class RigidMeshExporter : public MeshExporter
{
	friend class CExporter;

public:
	virtual bool exportTo(std::ostream& output, 
						  bool compressePos,
						  bool compressNormal,
						  bool compressTexcoord,
						  bool compressWeight) const;

    virtual STRING getTypeDesc() { return L"rigid"; }

private:
	STRING	m_attachedBoneName;
};

}

#endif
