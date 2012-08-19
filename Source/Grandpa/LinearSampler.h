#ifndef __GRP_LINEAR_SAMPLER_H__
#define __GRP_LINEAR_SAMPLER_H__

#include "AnimationSampler.h"

namespace grp
{

class LinearSampler : public AnimationSampler
{
public:
	template<typename T>
	static void sample(const VECTOR(TransformKey<T>)& keyFrames, float time, float fps, T& out);
};

}

#endif
