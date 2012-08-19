#include "Precompiled.h"
#include "AnimationExporter.h"
#include "ChunkFileIo.h"
#include "Spline.h"
#include "SplineSampler.h"

namespace grp
{

static const int CURRENT_VERSION = 0x0100;

extern unsigned long compressQuaternion(const Quaternion& q);

///////////////////////////////////////////////////////////////////////////////
AnimationExporter::AnimationExporter()
	: m_duration(0.0f)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AnimationExporter::exportTo(std::ostream& output, float fps, bool compressQuat) const
{
	//ANIM
	//	DURT
	//	TRKS
	//		TRAC
	//			NAME
	//			TRAN
	//			ROTA
	//			SCAL
	if (!createChunk(output, 'ANIM', sizeof(CURRENT_VERSION), (const char*)&CURRENT_VERSION))
	{
		return false;
	}
	size_t fileChunkSize = sizeof(CURRENT_VERSION);
	if (!createChunk(output, 'FMRT', sizeof(fps), (const char*)&fps))
	{
		return false;
	}
	fileChunkSize += (sizeof(fps) + CHUNK_HEADER_SIZE);

	if (!createChunk(output, 'DURT', sizeof(m_duration), (const char*)&m_duration))
	{
		return false;
	}
	fileChunkSize += (sizeof(m_duration) + CHUNK_HEADER_SIZE);

	if (!createChunk(output, 'SMPL', sizeof(m_sampleType), (const char*)&m_sampleType))
	{
		return false;
	}
	fileChunkSize += (sizeof(m_sampleType) + CHUNK_HEADER_SIZE);
	
	size_t trackCount = m_boneTracks.size();
	if (!createChunk(output, 'TRKS', sizeof(trackCount), (const char*)&trackCount))
	{
		return false;
	}
	size_t allTrackChunkSize = sizeof(trackCount);

	for (size_t i = 0; i < trackCount; ++i)
	{
		if (!createChunk(output, 'TRAC'))
		{
			return false;
		}
		const BoneTrack& boneTrack = m_boneTracks[i];
		//name
		size_t nameLength;
		if (!writeStringChunk(output, boneTrack.boneName, nameLength, 'NAME'))
		{
			return false;
		}
		size_t trackChunkSize = (nameLength + CHUNK_HEADER_SIZE);

		size_t keySize;
		//position keys
		if (!boneTrack.positionKeys.empty())
		{
			keySize = boneTrack.positionKeys.size() * sizeof(Vector3Key);
			if (!createChunk(output, 'TRAN', keySize, (const char*)&boneTrack.positionKeys[0]))
			{
				return false;
			}
			trackChunkSize += (keySize + CHUNK_HEADER_SIZE);
		}
		//rotation keys
		if (!boneTrack.rotationKeys.empty())
		{
			if (compressQuat)
			{
				if (!exportCompressedRotationsKeys(output, boneTrack.rotationKeys, keySize))
				{
					return false;
				}
				trackChunkSize += (keySize + CHUNK_HEADER_SIZE);
			}
			else
			{
				keySize = boneTrack.rotationKeys.size() * sizeof(QuaternionKey);
				if (!createChunk(output, 'ROTA', keySize, (const char*)&boneTrack.rotationKeys[0]))
				{
					return false;
				}
				trackChunkSize += (keySize + CHUNK_HEADER_SIZE);
			}
		}
		//scale keys
		if (!boneTrack.scaleKeys.empty())
		{
			keySize = boneTrack.scaleKeys.size() * sizeof(Vector3Key);
			if (!createChunk(output, 'SCAL', keySize, (const char*)&boneTrack.scaleKeys[0]))
			{
				return false;
			}
			trackChunkSize += (keySize + CHUNK_HEADER_SIZE);
		}
		if (!updateChunkSize(output, trackChunkSize))
		{
			return false;
		}
		allTrackChunkSize += (trackChunkSize + CHUNK_HEADER_SIZE);
	}
	if (!updateChunkSize(output, allTrackChunkSize))
	{
		return false;
	}
	fileChunkSize += (allTrackChunkSize + CHUNK_HEADER_SIZE);
	if (!updateChunkSize(output, fileChunkSize))
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationExporter::splineFitTracks(float positionThreshold, float rotationThreshold, float scaleThreshold)
{
	for (size_t i = 0; i < m_boneTracks.size(); ++i)
	{
		BoneTrack& boneTrack = m_boneTracks[i];
		if (boneTrack.positionKeys.size() > 1 && positionThreshold > 0.0f)
		{
			splineFitKeys(boneTrack.positionKeys, boneTrack.positionKnots, positionThreshold);
		}
		if (boneTrack.rotationKeys.size() > 1 && rotationThreshold > 0.0f)
		{
			splineFitKeys(boneTrack.rotationKeys, boneTrack.rotationKnots, rotationThreshold);
		}
		if (boneTrack.scaleKeys.size() > 1 && scaleThreshold > 0.0f)
		{
			splineFitKeys(boneTrack.scaleKeys, boneTrack.scaleKnots, scaleThreshold);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void AnimationExporter::clear()
{
	m_boneTracks.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool AnimationExporter::exportCompressedRotationsKeys(std::ostream& output, const VECTOR(QuaternionKey)& keys,
													  size_t& outputSize) const
{
	if (!createChunk(output, 'CROT'))
	{
		return false;
	}
	outputSize = keys.size() * (sizeof(float) + sizeof(unsigned long));
	for (size_t i = 0; i < keys.size(); ++i)
	{
		output.write((char*)&(keys[i].time), sizeof(float) );
		unsigned long compressed = compressQuaternion(keys[i].transform);
		output.write((char*)&compressed, sizeof(compressed));
	}
	return updateChunkSize(output, outputSize);
}

}
