#ifndef __GRP_ANIMATION_FILE_H__
#define __GRP_ANIMATION_FILE_H__

#include "ContentFile.h"
#include "IAnimation.h"
#include <string>
#include <vector>

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
struct TransformKey
{
	float	time;
	T		transform;
};

typedef TransformKey<Vector3> Vector3Key;
typedef TransformKey<Quaternion> QuaternionKey;

///////////////////////////////////////////////////////////////////////////////////////////////////
struct BoneTrack
{
	STRING					boneName;

	VECTOR(Vector3Key)		positionKeys;
	VECTOR(QuaternionKey)	rotationKeys;
	VECTOR(Vector3Key)		scaleKeys;

	VECTOR(Vector3)			positionKnots;
	VECTOR(Quaternion)		rotationKnots;
	VECTOR(Vector3)			scaleKnots;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class AnimationFile : public ContentFile
{
	friend class CExporter;

public:
	AnimationFile();
	
public:
	const VECTOR(BoneTrack)& getBoneTracks() const;

	float getDuration() const;

	bool importFrom(std::istream& input);

	AnimationSampleType getSampleType() const;

	float getFps() const;

private:
	void clear();

	template<typename KeyType, typename TransformType>
	void getSplineKnotsForKeys(VECTOR(KeyType)& keys, VECTOR(TransformType)& knots);

	void extract();

	bool importCompressedRotationKeys(std::istream& input, VECTOR(QuaternionKey)& keys, size_t keySize);

private:
	VECTOR(BoneTrack)	m_boneTracks;
	AnimationSampleType	m_sampleType;
	float				m_duration;
	float				m_fps;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(BoneTrack)& AnimationFile::getBoneTracks() const
{
	return m_boneTracks;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float AnimationFile::getDuration() const
{
	return m_duration;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename KeyType, typename TransformType>
void AnimationFile::getSplineKnotsForKeys(VECTOR(KeyType)& keys, VECTOR(TransformType)& knots)
{
	if (keys.size() < 2)
	{
		return;
	}
	VECTOR(TransformType) positions(keys.size());
	VECTOR(float) timeArray(keys.size());
	knots.resize((keys.size() - 1) * 2);
	for (size_t i = 0; i < keys.size(); ++i)
	{
		positions[i] = keys[i].transform;
		timeArray[i] = keys[i].time;
	}
	//cycle = true requires first key and last key are identical
	getSplineKnots(&positions[0], positions.size(), &(knots[0]), &timeArray[0], true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline AnimationSampleType AnimationFile::getSampleType() const
{
	return m_sampleType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float AnimationFile::getFps() const
{
	return m_fps;
}

}

#endif
