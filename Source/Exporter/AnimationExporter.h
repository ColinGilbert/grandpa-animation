#ifndef __GRP_ANIMATION_EXPORTER_H__
#define __GRP_ANIMATION_EXPORTER_H__

#include <string>
#include <vector>
#include "AnimationFile.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
class AnimationExporter
{
	friend class CExporter;

public:
	AnimationExporter();

public:
	const VECTOR(BoneTrack)& getBoneTracks() const;

	float getDuration() const;

	bool exportTo(std::ostream& output, float fps, bool compressQuat) const;

	AnimationSampleType getSampleType() const;

private:
	void clear();

	template<typename KeyType, typename TransformType>
	void splineFitKeys(VECTOR(KeyType)& keys, VECTOR(TransformType)& knots, float threshold);

	template<typename KeyType, typename TransformType>
	void getSplineKnotsForKeys(VECTOR(KeyType)& keys, VECTOR(TransformType)& knots);

	void splineFitTracks(float positionThreshold, float rotationThreshold, float scaleThreshold);

	bool exportCompressedRotationsKeys(std::ostream& output, const VECTOR(QuaternionKey)& keys,
										size_t& outputSize) const;

private:
	float				m_duration;
	VECTOR(BoneTrack)	m_boneTracks;
	AnimationSampleType	m_sampleType;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const VECTOR(BoneTrack)& AnimationExporter::getBoneTracks() const
{
	return m_boneTracks;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float AnimationExporter::getDuration() const
{
	return m_duration;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename KeyType, typename TransformType>
void AnimationExporter::splineFitKeys(VECTOR(KeyType)& keys, VECTOR(TransformType)& knots, float threshold)
{
	VECTOR(TransformType) positions(keys.size());
	VECTOR(float) times(keys.size());
	for (size_t i = 0; i < positions.size(); ++i)
	{
		positions[i] = keys[i].transform;
		times[i] = keys[i].time;
	}
	VECTOR(TransformType) outPositions(keys.size());
	knots.resize(keys.size() * 2);
	VECTOR(float) outTimes(keys.size());
	size_t outPointCount;
	splineFit(&positions[0], &times[0], positions.size(),
				&outPositions[0], &outTimes[0], outPointCount, &knots[0],
				threshold, false);
	keys.resize(outPointCount);
	knots.resize((outPointCount - 1) * 2);
	for (size_t i = 0; i < outPointCount; ++i)
	{
		keys[i].time = outTimes[i];
		keys[i].transform = outPositions[i];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename KeyType, typename TransformType>
void AnimationExporter::getSplineKnotsForKeys(VECTOR(KeyType)& keys, VECTOR(TransformType)& knots)
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
	getSplineKnots(&positions[0], positions.size(), &(knots[0]), &timeArray[0]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline AnimationSampleType AnimationExporter::getSampleType() const
{
	return m_sampleType;
}

}

#endif
