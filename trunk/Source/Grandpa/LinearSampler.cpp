#include "Precompiled.h"
#include "LinearSampler.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void LinearSampler::sample(const VECTOR(TransformKey<T>)& keyFrames, float time, float fps, T& out)
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

	int before = static_cast<int>(time * fps);
	int after = before + 1;
	assert(static_cast<size_t>(after) <= keyFrames.size() - 1);

	const TransformKey<T>& frameBefore = keyFrames[before];
	const TransformKey<T>& frameAfter = keyFrames[after];

	//assert(time >= frameBefore.time);
	float blendFactor = (time - frameBefore.time) / (frameAfter.time - frameBefore.time);
	if (blendFactor > 1.0f)
	{
		blendFactor = 1.0f;
	}
	out = frameBefore.transform.getLerp(frameAfter.transform, blendFactor);
}

template void LinearSampler::sample<Vector3>(const VECTOR(TransformKey<Vector3>)&, float, float, Vector3&);
template void LinearSampler::sample<Quaternion>(const VECTOR(TransformKey<Quaternion>)&, float, float, Quaternion&);

}
