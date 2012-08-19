#include "Precompiled.h"
#include "RigidMeshExporter.h"
#include "ChunkFileIo.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
bool RigidMeshExporter::exportTo(std::ostream& output,
								 bool compressePos,
								 bool compressNormal,
								 bool compressTexcoord,
								 bool compressWeight) const
{
	//MESH
	//ATCH
	size_t fileChunkSize;
	if (!exportMesh(output, &fileChunkSize, compressePos, compressNormal, compressTexcoord))
	{
		return false;
	}
	size_t nameChunkSize;
	if (!m_attachedBoneName.empty())
	{
		if (!writeStringChunk(output, m_attachedBoneName, nameChunkSize, 'ATCH'))
		{
			return false;
		}
		fileChunkSize += (nameChunkSize + CHUNK_HEADER_SIZE);
		if (!updateChunkSize(output, fileChunkSize))
		{
			return false;
		}
	}
	return true;
}

}
