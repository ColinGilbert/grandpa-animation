#include "Precompiled.h"
#include "AnimationFile.h"
#include "ChunkFileIo.h"
#include "Performance.h"
#include "Spline.h"
#include "SplineSampler.h"

namespace grp
{

static const int CURRENT_VERSION = 0x0100;

extern void decompressQuaternion(Quaternion& q, unsigned long compressed);
extern AnimationSampleType g_animationSampleType;

///////////////////////////////////////////////////////////////////////////////
AnimationFile::AnimationFile()
	: m_duration(0.0f)
	, m_fps(30.0f)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AnimationFile::importFrom(std::istream& input)
{
	//PERF_NODE_FUNC();

	clear();

	size_t fileSizeLeft;
	if (!findChunk(input, 'ANIM', fileSizeLeft))
	{
		return false;
	}

	int version;
	if (fileSizeLeft < sizeof(version))
	{
		return false;
	}
	input.read((char*)&version, sizeof(version));
	fileSizeLeft -= sizeof(version);

	if (!readChunk(input, 'FMRT', (char*)&m_fps, sizeof(m_fps), fileSizeLeft))
	{
		m_fps = 30.0f;
	}
	else
	{
		fileSizeLeft -= (sizeof(m_fps) + CHUNK_HEADER_SIZE);
	}
	if (!readChunk(input, 'DURT', (char*)&m_duration, sizeof(m_duration), fileSizeLeft))
	{
		return false;
	}
	fileSizeLeft -= (sizeof(m_duration) + CHUNK_HEADER_SIZE);

	if (!readChunk(input, 'SMPL',  (char*)&m_sampleType, sizeof(m_sampleType), fileSizeLeft))
	{
		m_sampleType = SAMPLE_SPLINE;
	}
	else
	{
		fileSizeLeft -= (sizeof(m_sampleType) + CHUNK_HEADER_SIZE);
	}
	if (m_sampleType > g_animationSampleType)
	{
		m_sampleType = g_animationSampleType;
	}

	size_t allTrackSizeLeft;
	if (!findChunk(input, 'TRKS', allTrackSizeLeft, fileSizeLeft))
	{
		return false;
	}
	size_t trackCount;
	input.read((char*)&trackCount, sizeof(trackCount));
	m_boneTracks.resize(trackCount);
	allTrackSizeLeft -= sizeof(trackCount);

	for (size_t i = 0; i < trackCount; ++i)
	{
		//PERF_NODE("load traks");

		BoneTrack& boneTrack = m_boneTracks[i];
		size_t trackSizeLeft;
		if (!findChunk(input, 'TRAC', trackSizeLeft, allTrackSizeLeft))
		{
			return false;
		}
		allTrackSizeLeft -= (trackSizeLeft + CHUNK_HEADER_SIZE);
		//name
		if (!readStringChunk(input, boneTrack.boneName, trackSizeLeft, 'NAME'))
		{
			return false;
		}
		size_t keySize;
		//position keys
		if (findChunk(input, 'TRAN', keySize, trackSizeLeft))
		{
			trackSizeLeft -= (keySize + CHUNK_HEADER_SIZE);
			if ((keySize % sizeof(Vector3Key)) != 0)
			{
				return false;
			}
			boneTrack.positionKeys.resize(keySize / sizeof(Vector3Key));
			input.read((char*)&boneTrack.positionKeys[0], keySize);
		}
		//rotation keys
		if (findChunk(input, 'ROTA', keySize, trackSizeLeft))
		{
			trackSizeLeft -= (keySize + CHUNK_HEADER_SIZE);
			if ((keySize % sizeof(QuaternionKey)) != 0)
			{
				return false;
			}
			boneTrack.rotationKeys.resize(keySize / sizeof(QuaternionKey));
			input.read((char*)&boneTrack.rotationKeys[0], keySize);
		}
		else if (findChunk(input, 'CROT', keySize, trackSizeLeft))
		{
			trackSizeLeft -= (keySize + CHUNK_HEADER_SIZE);
			if (!importCompressedRotationKeys(input, boneTrack.rotationKeys, keySize))
			{
				return false;
			}
		}
		//scale keys
		if (findChunk(input, 'SCAL', keySize, trackSizeLeft))
		{
			trackSizeLeft -= (keySize + CHUNK_HEADER_SIZE);
			if ((keySize % sizeof(Vector3Key)) != 0)
			{
				return false;
			}
			boneTrack.scaleKeys.resize(keySize / sizeof(Vector3Key));
			input.read((char*)&boneTrack.scaleKeys[0], keySize);
		}
	}
	extract();
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationFile::clear()
{
	m_boneTracks.clear();
}

void AnimationFile::extract()
{
	//PERF_NODE_FUNC();

	for (size_t i = 0; i < m_boneTracks.size(); ++i)
	{
		BoneTrack& boneTrack = m_boneTracks[i];

		getSplineKnotsForKeys(boneTrack.positionKeys, boneTrack.positionKnots);
		getSplineKnotsForKeys(boneTrack.rotationKeys, boneTrack.rotationKnots);
		getSplineKnotsForKeys(boneTrack.scaleKeys, boneTrack.scaleKnots);

		if (m_sampleType == SAMPLE_LINEAR
			|| m_sampleType == SAMPLE_STEP)
		{
			size_t frameCount = static_cast<size_t>(m_duration * m_fps) + 1;
			if (boneTrack.positionKeys.size() > 1)
			{
				VECTOR(Vector3Key) keys(frameCount);
				for (size_t i = 0; i < frameCount; ++i)
				{
					float& time = keys[i].time;
					time = i / m_fps;
					if (time > boneTrack.positionKeys.back().time)
					{
						time = boneTrack.positionKeys.back().time;
					}
					SplineSampler::sample(boneTrack.positionKeys, boneTrack.positionKnots, time, keys[i].transform);
				}
				boneTrack.positionKeys.swap(keys);
			}
			if (boneTrack.rotationKeys.size() > 1)
			{
				VECTOR(QuaternionKey) keys(frameCount);
				for (size_t i = 0; i < frameCount; ++i)
				{
					float& time = keys[i].time;
					time = i / m_fps;
					if (time > boneTrack.rotationKeys.back().time)
					{
						time = boneTrack.rotationKeys.back().time;
					}
					SplineSampler::sample(boneTrack.rotationKeys, boneTrack.rotationKnots, time, keys[i].transform);
				}
				boneTrack.rotationKeys.swap(keys);
			}
			if (boneTrack.scaleKeys.size() > 1)
			{
				VECTOR(Vector3Key) keys(frameCount);
				for (size_t i = 0; i < frameCount; ++i)
				{
					float& time = keys[i].time;
					time = i / m_fps;
					if (time > boneTrack.scaleKeys.back().time)
					{
						time = boneTrack.scaleKeys.back().time;
					}
					SplineSampler::sample(boneTrack.scaleKeys, boneTrack.scaleKnots, time, keys[i].transform);
				}
				boneTrack.scaleKeys.swap(keys);
			}
			boneTrack.positionKnots.clear();
			boneTrack.rotationKnots.clear();
			boneTrack.scaleKnots.clear();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AnimationFile::importCompressedRotationKeys(std::istream& input, VECTOR(QuaternionKey)& keys, size_t keySize)
{
	size_t stride = sizeof(float) + sizeof(unsigned long);
	if ((keySize % stride) != 0)
	{
		return false;
	}
	size_t keyCount = keySize / stride;
	keys.resize(keyCount);
	for (size_t i = 0; i < keyCount; ++i)
	{
		QuaternionKey& key = keys[i];
		input.read((char*)&(key.time), sizeof(float));
		unsigned long compressed;
		input.read((char*)&compressed, sizeof(unsigned long));
		decompressQuaternion(key.transform, compressed);
	}
	return true;
}

}
