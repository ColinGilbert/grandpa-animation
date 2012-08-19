#ifndef __GRP_ANIMATION_SAMPLER_H__
#define __GRP_ANIMATION_SAMPLER_H__

namespace grp
{

class AnimationSampler
{
protected:
	template<typename T>
	static int getKeyFrameByTime(const VECTOR(T)& keyFrames, float time)
	{
		int min = 0;
		int max = static_cast<int>(keyFrames.size() - 1);

		while (min < max - 1)
		{
			int middle = (min + max) / 2;
			if (time < keyFrames[middle].time)
			{
				max = middle;
			}
			else
			{
				min = middle;
			}
		}
		return min;
	}

};

}

#endif
