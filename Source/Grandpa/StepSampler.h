#ifndef __GRP_STEP_SAMPLER_H__
#define __GRP_STEP_SAMPLER_H__

#include "AnimationSampler.h"

namespace grp
{

class StepSampler : public AnimationSampler
{
public:
	template<typename T>
	static void sample(const VECTOR(TransformKey<T>)& keyFrames, float time, float fps, T& out);
};

}

#endif
