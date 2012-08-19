#include "Precompiled.h"
#include "StepSampler.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
void StepSampler::sample(const VECTOR(TransformKey<T>)& keyFrames, float time, float fps, T& out)
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
	out = keyFrames[before].transform;
}

template void StepSampler::sample<Vector3>(const VECTOR(TransformKey<Vector3>)&, float, float, Vector3&);
template void StepSampler::sample<Quaternion>(const VECTOR(TransformKey<Quaternion>)&, float, float, Quaternion&);

}
