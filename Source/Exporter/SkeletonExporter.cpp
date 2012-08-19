#include "Precompiled.h"
#include "SkeletonExporter.h"
#include "ChunkFileIo.h"

namespace grp
{

static const int CURRENT_VERSION = 0x0100;

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SkeletonExporter::exportTo(std::ostream& output) const
{
	//SKEL
	//	BONS
	//		BONE
	//			NAME
	//			DATA id, parentId, default transforms
	if (!createChunk(output, 'SKEL', sizeof(CURRENT_VERSION), (const char*)&CURRENT_VERSION))
	{
		return false;
	}
	size_t fileChunkSize = sizeof(CURRENT_VERSION);

	size_t boneCount = m_coreBones.size();
	if (!createChunk(output, 'BONS', sizeof(boneCount), (const char*)&boneCount))
	{
		return false;
	}
	size_t allBoneChunkSize = sizeof(boneCount);

	for (size_t i = 0; i < boneCount; ++i)
	{
		if (!createChunk(output, 'BONE'))
		{
			return false;
		}
		const CoreBone& bone = m_coreBones[i];
		//name
		size_t nameLength;
		if (!writeStringChunk(output, bone.name, nameLength, 'NAME'))
		{
			return false;
		}
		size_t boneChunkSize = nameLength + CHUNK_HEADER_SIZE;
		//data
		if (!createChunk(output, 'DATA'))
		{
			return false;
		}
		assert(bone.parentId < static_cast<int>(boneCount));
		output.write((char*)(&(bone.parentId)), sizeof(int));
		output.write((char*)(&(bone.position)), sizeof(Vector3));
		output.write((char*)(&(bone.rotation)), sizeof(Quaternion));
		output.write((char*)(&(bone.scale)), sizeof(Vector3));
		size_t dataChunkSize = sizeof(int) + sizeof(Vector3) + sizeof(Quaternion) + sizeof(Vector3);
		if (!updateChunkSize(output, dataChunkSize))
		{
			return false;
		}
		boneChunkSize += (dataChunkSize + CHUNK_HEADER_SIZE);
		//user property
		if (bone.property != L"")
		{
			size_t propertyLength;
			if (!writeStringChunk(output, bone.property, propertyLength, 'PROP'))
			{
				return false;
			}
			boneChunkSize += (propertyLength + CHUNK_HEADER_SIZE);
		}
		if (!updateChunkSize(output, boneChunkSize))
		{
			return false;
		}
		allBoneChunkSize += (boneChunkSize + CHUNK_HEADER_SIZE);
	}
	if (!updateChunkSize(output, allBoneChunkSize))
	{
		return false;
	}
	fileChunkSize += (allBoneChunkSize + CHUNK_HEADER_SIZE);
	if (!updateChunkSize(output, fileChunkSize))
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SkeletonExporter::clear()
{
	m_boneNameMap.clear();
	m_coreBones.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int SkeletonExporter::addCoreBone(const CoreBone& bone)
{
	m_coreBones.push_back(bone);

	CoreBone& boneAdded = m_coreBones.back();
	int id = static_cast<int>(m_coreBones.size()) - 1;
	boneAdded.parentId = -1;
	boneAdded.childrenId.clear();
	m_boneNameMap.insert(make_pair(boneAdded.name, id));
	return id;
}

}
