#ifndef __GRP_SPLINE_FUNCTIONS_H__
#define __GRP_SPLINE_FUNCTIONS_H__

namespace grp
{
//this function is not well optimized, because it's not suppose to be call every frame

//type T can be Vector2, Vector3, Vector4
//size of knots should be greater than (pointCount - 1) * 2
//if cycle == true, first point and last point should be the same
template<typename T>
void getSplineKnots(const T* points, size_t pointCount, T* knots, float* timeArray = NULL, bool cycle = false)
{
	if (cycle && (pointCount < 4 || points[0] != points[pointCount - 1]))
	{
		cycle = false;
	}
	if (pointCount <= 1)
	{
		return;
	}
	if (pointCount == 2)
	{
		knots[0] = points[0] + (points[1] - points[0]) / 3.0f;
		knots[1] = points[0] + (points[1] - points[0]) * 2.0f / 3.0f;
		//knots[0] = points[0].getLerp(points[1], 1.0f / 3.0f);
		//knots[1] = points[0].getLerp(points[1], 2.0f / 3.0f);
		return;
	}

	for (size_t i = 0; i <= pointCount - 3; ++i)
	{
		const T& s0 = points[i];
		const T& s1 = points[i+1];
		const T& s2 = points[i+2];

		const T& b0 = s0;
		T b1 = (s1 * 6.0f - s0 - s2) / 4.0f;
		const T& b2 = s2;

		if (timeArray != NULL)
		{
			float t0 = timeArray[i+1] - timeArray[i];
			float t2 = timeArray[i+2] - timeArray[i+1];
			float sum = t0 + t2;
			if (sum < 0.0001f)
			{
				b1 = (s1 * 6.0f - s0 - s2) / 4.0f;
			}
			else
			{
				b1 = (s1 * 3.0f - (s0 * t2 + s2 * t0) / sum) / 2.0f;
			}
		}
		else
		{
			b1 = (s1 * 6.0f - s0 - s2) / 4.0f;
		}

		if (i == 0)
		{
			knots[2*i] = (b0 + (b1 - b0) / 3.0f);
		}
		knots[2*i+1] = (b0 + (b1 - b0) * 2.0f / 3.0f);
		knots[2*i+2] = (b1 + (b2 - b1) / 3.0f);
		if (i == pointCount - 3)
		{
			knots[2*i+3] = (b1 + (b2 - b1) * 2.0f / 3.0f);
		}
	}
	if (cycle)
	{
		if (pointCount >= 3)
		{
			const T& s0 = points[pointCount-2];
			const T& s1 = points[pointCount-1];
			const T& s2 = points[1];

			T t0 = s1 * 6.0f - s0 - s2;

			T b0 = s0;
			T b1 = t0 / 4.0f;
			T b2 = s2;

			knots[2*(pointCount-1)-1] = (b0 + (b1 - b0) * 2.0f / 3.0f);
			knots[0] = (b1 + (b2 - b1) / 3.0f);
		}
	}
}

//return index of point with max error
template<typename T>
void getMaxErrorPoint(const T* currentKnots, const size_t* currentPointIndices, size_t currentPointCount,
					  const T* originPoints, const float* originTimeArr, size_t originPointCount,
					  float threshold, size_t& maxErrorIndex, size_t& insertIndex, float& maxError)
{
	maxError = 0.0f;

	for (size_t i = 0; i < currentPointIndices[0]; ++i)
	{
		float error = originPoints[i].distance(originPoints[currentPointIndices[0]]);
		if (error > maxError)
		{
			maxError = error;
			maxErrorIndex = i;
			insertIndex = 0;
		}
	}
	for (size_t i = 0; i < currentPointCount - 1; ++i)
	{
		for (size_t j = currentPointIndices[i] + 1; j < currentPointIndices[i+1]; ++j)
		{
			float t;
			if (originTimeArr != NULL)
			{
				t = (originTimeArr[j] - originTimeArr[currentPointIndices[i]])
					/ (originTimeArr[currentPointIndices[i+1]] - originTimeArr[currentPointIndices[i]]);
			}
			else
			{
				t = float(j - currentPointIndices[i]) / (currentPointIndices[i+1] - currentPointIndices[i]);
			}
			T samplePoint = sampleCubicBezier(originPoints[currentPointIndices[i]],
												currentKnots[i*2],
												currentKnots[i*2+1],
												originPoints[currentPointIndices[i+1]],
												t);
			float error = originPoints[j].distance(samplePoint);
			if (error > maxError)
			{
				maxError = error;
				maxErrorIndex = j;
				insertIndex = i + 1;
			}
		}
	}
	for (size_t i = currentPointIndices[currentPointCount-1] + 1; i < originPointCount; ++i)
	{
		float error = originPoints[i].distance(originPoints[currentPointIndices[currentPointCount-1]]);
		if (error > maxError)
		{
			maxError = error;
			maxErrorIndex = i;
			insertIndex = currentPointCount;
		}
	}
}

//only pick from input points
template<typename T>
void splineFit(const T* inPoints, const float* inTimeArr, size_t inPointCount,
				T* outPoints, float* outTimeArr, size_t& outPointCount,
				T* outKnots, float threshold, bool cycle = false)
{
	if (inPointCount == 0)
	{
		outPointCount = 0;
		return;
	}

	VECTOR(size_t) currentPointIndices;
	VECTOR(T) currentPoints;
	VECTOR(float) currentTimeArr;
	VECTOR(T) currentKnots;

	size_t maxErrorIndex = 0;
	size_t insertIndex = 0;
	float maxError;
	do
	{
		currentPointIndices.insert(currentPointIndices.begin() + insertIndex, maxErrorIndex);
		currentPoints.insert(currentPoints.begin() + insertIndex, inPoints[maxErrorIndex]);
		currentTimeArr.insert(currentTimeArr.begin() + insertIndex, inTimeArr[maxErrorIndex]);
		currentKnots.resize(currentPoints.size() * 2);

		getSplineKnots(&currentPoints[0], currentPoints.size(), &currentKnots[0], &currentTimeArr[0], cycle);

		getMaxErrorPoint(&currentKnots[0], &currentPointIndices[0], currentPoints.size(),
						inPoints, inTimeArr, inPointCount, threshold, maxErrorIndex, insertIndex, maxError);
	} while (maxError > threshold);

	outPointCount = currentPointIndices.size();
	for (size_t i = 0; i < outPointCount; ++i)
	{
		*(outPoints++) = currentPoints[i];
		*(outTimeArr++) = inTimeArr[currentPointIndices[i]];
		*(outKnots++) = currentKnots[i*2];
		*(outKnots++) = currentKnots[i*2+1];
	}
}

}

#endif
