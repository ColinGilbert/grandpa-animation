#ifndef __GRP_SPLINE_SAMPLER_H__
#define __GRP_SPLINE_SAMPLER_H__

#include "AnimationSampler.h"
#include "AnimationFile.h"

namespace grp
{

class SplineSampler : public AnimationSampler
{
public:
	template<typename T>
	static void sample(const VECTOR(TransformKey<T>)& keyFrames, const VECTOR(T)& knots, float time, T& out);
};

}

#endif
