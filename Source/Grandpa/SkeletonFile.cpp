#include "Precompiled.h"
#include "SkeletonFile.h"
#include "ChunkFileIo.h"
#include "Performance.h"

namespace grp
{

static const int CURRENT_VERSION = 0x0100;

///////////////////////////////////////////////////////////////////////////////////////////////////
bool SkeletonFile::importFrom(std::istream& input)
{
	//PERF_NODE_FUNC();

	clear();

	size_t fileSizeLeft;
	if (!findChunk(input, 'SKEL', fileSizeLeft))
	{
		return false;
	}

	int version;
	if (fileSizeLeft < sizeof(version))
	{
		return false;
	}
	input.read((char*)&version, sizeof(version));
	fileSizeLeft -= sizeof(version);

	size_t allBoneSizeLeft;
	if (!findChunk(input, 'BONS', allBoneSizeLeft))
	{
		return false;
	}
	size_t boneCount;
	input.read((char*)(&boneCount), sizeof(boneCount));
	allBoneSizeLeft -= sizeof(boneCount);
	
	m_coreBones.resize(boneCount);
	for (size_t i = 0; i < boneCount; ++i)
	{
		CoreBone& bone = m_coreBones[i];
		size_t boneSizeLeft;
		if (!findChunk(input, 'BONE', boneSizeLeft, allBoneSizeLeft))
		{
			return false;
		}
		allBoneSizeLeft -= (boneSizeLeft + CHUNK_HEADER_SIZE);
		//name
		if (!readStringChunk(input, bone.name, boneSizeLeft, 'NAME'))
		{
			return false;
		}
		//data
		size_t dataSize;
		if (!findChunk(input, 'DATA', dataSize, boneSizeLeft))
		{
			return false;
		}
		if (dataSize != sizeof(int) + sizeof(Vector3) + sizeof(Quaternion) + sizeof(Vector3))
		{
			return false;
		}
		input.read((char*)(&(bone.parentId)), sizeof(int));
		if (bone.parentId >= static_cast<int>(boneCount))
		{
			return false;
		}
		input.read((char*)(&(bone.position)), sizeof(Vector3));
		input.read((char*)(&(bone.rotation)), sizeof(Quaternion));
		input.read((char*)(&(bone.scale)), sizeof(Vector3));

		boneSizeLeft -= dataSize;

		readStringChunk(input, bone.property, boneSizeLeft, 'PROP');

		m_boneNameMap[bone.name] = i;
	}
	//add children id
	for (size_t i = 0; i < boneCount; ++i)
	{
		CoreBone& bone = m_coreBones[i];
		if (bone.parentId >= 0)
		{
			m_coreBones[bone.parentId].childrenId.push_back(static_cast<int>(i));
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void SkeletonFile::clear()
{
	m_boneNameMap.clear();
	m_coreBones.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int SkeletonFile::addCoreBone(const CoreBone& bone)
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
