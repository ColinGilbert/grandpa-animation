#include "Precompiled.h"
#include "RigidMeshFile.h"
#include "ChunkFileIo.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
bool RigidMeshFile::importFrom(std::istream& input)
{
	size_t fileSizeLeft;
	if (!importMesh(input, &fileSizeLeft))
	{
		return false;
	}
	readStringChunk(input, m_attachedBoneName, fileSizeLeft, 'ATCH');
	return true;
}

}
