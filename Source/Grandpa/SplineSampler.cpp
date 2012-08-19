#include "Precompiled.h"
#include "SplineSampler.h"
#include "Bezier.h"
#include "AnimationFile.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void SplineSampler::sample(const VECTOR(TransformKey<T>)& keyFrames,
							const VECTOR(T)& knots,
							float time,
							T& out)
{
	assert(!keyFrames.empty());
	
	if (time <= keyFrames.front().time)
	{
		out = keyFrames.front().transform;
		return;
	}
	if (time >= keyFrames.back().time)
	{
		out = keyFrames.back().transform;
		return;
	}
	assert(keyFrames.size() > 1);

	int before = getKeyFrameByTime(keyFrames, time);
	int after = before + 1;
	assert(static_cast<size_t>(after) <= keyFrames.size() - 1);

	const TransformKey<T>& frameBefore = keyFrames[before];
	const TransformKey<T>& frameAfter = keyFrames[after];

	assert(time >= frameBefore.time);
	float blendFactor = (time - frameBefore.time) / (frameAfter.time - frameBefore.time);
	if (blendFactor > 1.0f)
	{
		blendFactor = 1.0f;
	}

	out = grp::sampleCubicBezier(frameBefore.transform,
									knots[2 * before],
									knots[2 * before + 1],
									frameAfter.transform,
									blendFactor);
}

template void SplineSampler::sample<Vector3>(const VECTOR(TransformKey<Vector3>)&, const VECTOR(Vector3)&, float, Vector3&);
template void SplineSampler::sample<Quaternion>(const VECTOR(TransformKey<Quaternion>)&, const VECTOR(Quaternion)&, float, Quaternion&);

}
