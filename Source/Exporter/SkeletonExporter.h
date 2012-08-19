#ifndef __GRP_SKELETON_EXPORTER_H__
#define __GRP_SKELETON_EXPORTER_H__

#include <string>
#include <vector>
#include <map>

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
struct CoreBone
{
	STRING		name;
	STRING		property;
	int			parentId;
	VECTOR(int)	childrenId;
	Vector3		position;
	Quaternion	rotation;
	Vector3		scale;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class SkeletonExporter
{
	friend class CExporter;

public:
	CoreBone* getCoreBone(int id);
	CoreBone* getCoreBone(const STRING& name);

	int getBoneId(const STRING& name) const;

	const VECTOR(CoreBone)& getCoreBones() const;

	bool exportTo(std::ostream& output) const;

private:
	void clear();

	int addCoreBone(const CoreBone& bone);

private:
	VECTOR(CoreBone)	m_coreBones;
	MAP(STRING, int)	m_boneNameMap;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline CoreBone* SkeletonExporter::getCoreBone(int id)
{
	if (id < 0 || id >= static_cast<int>(m_coreBones.size()))
	{
		return NULL;
	}
	return &(m_coreBones[id]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline CoreBone* SkeletonExporter::getCoreBone(const STRING& name)
{
	MAP(STRING, int)::const_iterator found = m_boneNameMap.find(name);
	if (found == m_boneNameMap.end())
	{
		return NULL;
	}
	return getCoreBone((*found).second);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline int SkeletonExporter::getBoneId(const STRING& name) const
{
	MAP(STRING, int)::const_iterator found;
	found = m_boneNameMap.find(name);
	if (found == m_boneNameMap.end())
	{
		return -1;
	}
	return (*found).second;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(CoreBone)& SkeletonExporter::getCoreBones() const
{
	return m_coreBones;
}

}

#endif
