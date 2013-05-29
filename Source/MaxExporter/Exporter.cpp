///////////////////////////////////////////////////////////////////////////////
//Exporter.cpp
//
//描述：
//
//历史：
//		07-12-17	创建
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.h"
#include "Exporter.h"
#include "SkeletonExporter.h"
#include "AnimationExporter.h"
#include "MeshExporter.h"
#include "MathConversion.h"
#include "OptionDlg.h"
#include "StrSafe.h"
#include "ISkin.h"
#include <fstream>
#include <string>
#include <map>

extern HINSTANCE g_hInstance;

using std::vector;
using std::map;
using std::wstring;
using std::make_pair;
using std::fstream;
using std::ios_base;

using grp::MeshExporter;
using grp::AnimationExporter;
using grp::SkeletonExporter;
using grp::CoreBone;
using grp::BoneTrack;
using grp::Vector3Key;
using grp::QuaternionKey;
using grp::Vector3;
using grp::Vector4;
using grp::Matrix;
using grp::Quaternion;

///////////////////////////////////////////////////////////////////////////////
DWORD WINAPI NullFunc(LPVOID arg)
{
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
CExporter::CExporter(Interface* pInterface)
	:	m_interface(pInterface)
	,	m_skeleton(NULL)
	,	m_animation(NULL)
	,	m_lastError("未定义错误")
{
}

///////////////////////////////////////////////////////////////////////////////
CExporter::~CExporter()
{
	if (m_skeleton != NULL)
	{
		delete m_skeleton;
		m_skeleton = NULL;
	}
	if (m_animation != NULL)
	{
		delete m_animation;
		m_animation = NULL;
	}
	for (DWORD i = 0; i < m_meshes.size(); ++i)
	{
		MeshExporter* mesh = m_meshes[i];
		//assert(mesh != NULL);
		delete mesh;
	}
}

///////////////////////////////////////////////////////////////////////////////
bool CExporter::doExport(LPCWSTR szFilename)
{
	assert(m_interface != NULL);
	INode* pNodeRoot = m_interface->GetRootNode();
	if (NULL == pNodeRoot)
	{
		setLastError("没找到根节点");
		return false;
	}

	EnumTree(pNodeRoot);

	m_animationBoneNodes = m_skeletonBoneNodes;

	m_options.exportType = ::GetPrivateProfileIntW(L"grandpa exporter", L"type", DEFAULT_SETTINGS, L"GrandpaMax.ini");
	m_options.skeletonBoneNodes = &m_skeletonBoneNodes;
	m_options.animationBoneNodes = &m_animationBoneNodes;
	m_options.meshNodes = &m_meshNodes;

	wchar_t profileString[32];
	::GetPrivateProfileStringW(L"grandpa exporter", L"position tolerance", L"0.001", profileString, sizeof(profileString), L"GrandpaMax.ini");
	swscanf(profileString, L"%f", &m_options.positionTolerance);
	::GetPrivateProfileStringW(L"grandpa exporter", L"rotation tolerance", L"0.001", profileString, sizeof(profileString), L"GrandpaMax.ini");
	swscanf(profileString, L"%f", &m_options.rotationTolerance);
	::GetPrivateProfileStringW(L"grandpa exporter", L"scale tolerance", L"0.001", profileString, sizeof(profileString), L"GrandpaMax.ini");
	swscanf(profileString, L"%f", &m_options.scaleTolerance);
	::GetPrivateProfileStringW(L"grandpa exporter", L"lod level scale", L"0.4", profileString, sizeof(profileString), L"GrandpaMax.ini");
	swscanf(profileString, L"%f", &m_options.lodLevelScale);
	::GetPrivateProfileStringW(L"grandpa exporter", L"lod max error", L"0.1", profileString, sizeof(profileString), L"GrandpaMax.ini");
	swscanf(profileString, L"%f", &m_options.lodMaxError);

	if (!::DialogBoxParam(g_hInstance,
						  MAKEINTRESOURCE(IDD_PANEL), 
						  GetActiveWindow(), 
						  OptionDlgProc,
						  (LPARAM)(&m_options)))
	{
		return true;
	}

	wstring strFilename = szFilename;
	//去掉'.'
	assert(strFilename.length() >= 1);
	strFilename.erase(strFilename.length() - 1, 1);

    m_mainFileName = getFilenameFromFullpath( strFilename );
    m_exportPath = getFilePath( strFilename );

	if ((m_options.exportType & EXP_SKELETON) != 0 && !ExportSkeleton(strFilename))
	{
		return false;
	}
	if ((m_options.exportType & EXP_MESH) != 0 && !exportMesh(strFilename))
	{
		return false;
	}
	if ((m_options.exportType & EXP_ANIMATION) != 0 && !ExportAnimation(strFilename))
	{
		return false;
	}

    if( !ExportMaterial(strFilename))
    {
        return false;
    }

    if( !ExportPart())
    {
        return false;
    }

    if( !ExportModel())
    {
        return false;
    }

    ::MessageBeep(MB_OK);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
void CExporter::EnumTree(INode* pNode)
{
	assert(pNode != NULL);

	ENUM_NODE_TYPE type = checkNodeType(pNode);
	switch (type)
	{
	case ENUM_NODE_BONE:
	case ENUM_NODE_DUMMY:
		m_skeletonBoneNodes.push_back(pNode);
		break;

	case ENUM_NODE_MESH:
		m_meshNodes.push_back(pNode);
		break;
	}

	//递归子节点
	int iNumChild = pNode->NumberOfChildren();
	for (int i = 0; i < iNumChild; ++i)
	{
		EnumTree(pNode->GetChildNode(i));
	}
}

///////////////////////////////////////////////////////////////////////////////
bool CExporter::ExportSkeleton(const wstring& strFilename)
{
	if (!BuildSkeleton())
	{
		return false;
	}

	wstring strFilePath = strFilename + FILE_EXT_SKELETON;
	fstream file;
	file.open(strFilePath.c_str(), ios_base::out | ios_base::binary);
	if (!file.is_open())
	{
		char szError[1024];
		::StringCchPrintf(szError, sizeof(szError), "Failed to open file [%s]", strFilePath.c_str());
		setLastError(szError);
		return false;
	}
	assert(m_skeleton != NULL);
	if (!m_skeleton->exportTo(file))
	{
		file.close();
		return false;
	}
	file.close();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool CExporter::ExportAnimation(const wstring& strFilename)
{
	if (!BuildAnimation())
	{
		return false;
	}

	wstring strFilePath = strFilename + FILE_EXT_ANIMATION;
	fstream file;
	file.open(strFilePath.c_str(), ios_base::out | ios_base::binary);
	if (!file.is_open())
	{
		char szError[1024];
		::StringCchPrintf(szError, sizeof(szError), "Failed to open file [%s]", strFilePath.c_str());
		setLastError(szError);
		return false;
	}
	assert(m_animation != NULL);
	if (!m_animation->exportTo(file, (float)GetFrameRate(), (m_options.exportType & EXP_ANIM_COMPRESS_QUATERNION) != 0))
	{
		file.close();
		return false;
	}
	file.close();
	return true;
}

///////////////////////////////////////////////////////////////////////////////
CExporter::ENUM_NODE_TYPE CExporter::checkNodeType(INode* pNode)
{
	assert(pNode != NULL);
	if (pNode->GetParentNode() == NULL)
	{
		return ENUM_NODE_ROOT;
	}
	ObjectState os;
	os = pNode->EvalWorldState(0);

	if (NULL == os.obj)
	{
		return ENUM_NODE_DUMMY;
	}
	if (os.obj->ClassID() == Class_ID(DUMMY_CLASS_ID, 0))
	{
		return ENUM_NODE_DUMMY;
	}
#if MAX_RELEASE >= 4000
	if (os.obj->ClassID() == BONE_OBJ_CLASSID)
	{
		return ENUM_NODE_BONE;
	}
#endif
	Control *pControl;
	pControl = pNode->GetTMController();
	if ((pControl->ClassID() == BIPSLAVE_CONTROL_CLASS_ID)
		|| (pControl->ClassID() == BIPBODY_CONTROL_CLASS_ID))
	{
		return ENUM_NODE_BONE;
	}
	if (pControl->ClassID() == FOOTPRINT_CLASS_ID)
	{
		return ENUM_NODE_UNKNOWN;
	}
	if (os.obj->SuperClassID() == GEOMOBJECT_CLASS_ID)
	{
		return ENUM_NODE_MESH;
	}
	return ENUM_NODE_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////
void CExporter::getBoneTransform(INode* pNode, CoreBone& bone)
{
	assert(pNode != NULL);
	assert(m_interface != NULL);
	Matrix3 mNode = pNode->GetNodeTM(m_interface->GetTime());
	Matrix3 mRelative;
	
	if (pNode->IsRootNode())
	{
		mRelative = mNode;
	}
	else
	{
		Matrix3 mParent = pNode->GetParentTM(m_interface->GetTime());
        if (memcmp(&mNode, &mParent, sizeof(Matrix3)) == 0)
        {
            mRelative.IdentityMatrix();
        }
        else
        {
            mRelative = mNode * Inverse(mParent);
        }
	}
	::GetTransformFromMatrix3(mRelative,
							   bone.position,
							   bone.rotation,
							   bone.scale);
}

///////////////////////////////////////////////////////////////////////////////
bool CExporter::BuildSkeleton()
{
	if (m_skeleton != NULL)
	{
		delete m_skeleton;
	}
	m_skeleton = new SkeletonExporter;

	map<INode*, int> mapNodeBone;

	//添加骨骼
	for (vector<INode*>::iterator iterNode = m_skeletonBoneNodes.begin();
		iterNode != m_skeletonBoneNodes.end();
		++iterNode)
	{
		INode* pNode = *iterNode;
		assert(pNode != NULL);

		CoreBone bone;
		wchar_t unicodeString[256];
		mbstowcs(unicodeString, pNode->GetName(), 255);
		bone.name = unicodeString;
		getBoneTransform(pNode, bone);
		if (fabs(bone.scale.X - 1.0f) < 0.001f
			&& fabs(bone.scale.Y - 1.0f) < 0.001f
			&& fabs(bone.scale.Z - 1.0f) < 0.001f)
		{
			bone.scale = Vector3(1.0f, 1.0f, 1.0f);
		}
		if ((m_options.exportType & EXP_SKEL_PROPERTY) != 0)
		{
			TSTR buffer;
			pNode->GetUserPropBuffer(buffer);
			wchar_t* unicodeString = new wchar_t[buffer.length() + 1];
			mbstowcs(unicodeString, buffer.data(), buffer.length() + 1);
			bone.property = unicodeString;
			delete[] unicodeString;
		}
		int iBoneId = m_skeleton->addCoreBone(bone);
		mapNodeBone[pNode] = iBoneId;
	}
	//骨架结构
	for (vector<INode*>::iterator iterNode = m_skeletonBoneNodes.begin();
		iterNode != m_skeletonBoneNodes.end();
		++iterNode)
	{
		INode* pNode = *iterNode;
		assert(pNode != NULL);

		map<INode*, int>::iterator iterFound = mapNodeBone.find(pNode);
		assert(iterFound != mapNodeBone.end());
		int boneId = iterFound->second;
		CoreBone* bone = m_skeleton->getCoreBone(boneId);
		assert(bone != NULL);
		//assert(bone->Id == (*iterFound).second);

		INode* parentNode = pNode->GetParentNode();
		if (NULL == parentNode)
		{
			continue;
		}
		iterFound = mapNodeBone.find(parentNode);
		if (iterFound == mapNodeBone.end())
		{
			//虽然父亲也是骨骼，但可能用户指定了不需要输出，所以不认为是出错
			continue;
		}
		int parentId = iterFound->second;
		CoreBone* parentBone = m_skeleton->getCoreBone(parentId);
		assert(parentBone != NULL);

		bone->parentId = parentId;
		parentBone->childrenId.push_back(boneId);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool CExporter::BuildAnimation()
{
	if (m_animation != NULL)
	{
		delete m_animation;
	}
	m_animation = new AnimationExporter;

	assert(m_interface != NULL);
	Interval interval = m_interface->GetAnimRange();
	int iTimeStart = interval.Start();
	int iTimeEnd = interval.End();
	int iFrameRate = GetFrameRate();
	int iFrameTime = GetTicksPerFrame();
	m_animation->m_duration = float(iTimeEnd - iTimeStart) / iFrameTime / iFrameRate;

	size_t dwNumBone = m_animationBoneNodes.size();
	m_animation->m_boneTracks.resize(dwNumBone);

	DWORD dwFrameNum = (iTimeEnd - iTimeStart) / iFrameTime + 1;

	for (DWORD i = 0; i < dwNumBone; ++i)
	{
		if (!buildBoneTrack(m_animationBoneNodes[i],
							  m_animation->m_boneTracks[i],
							  iTimeStart,
							  iTimeEnd,
							  iFrameRate,
							  iFrameTime))
		{
			return false;
		}
	}
	m_animation->splineFitTracks(m_options.positionTolerance,
								  m_options.rotationTolerance,
								  m_options.scaleTolerance);
	switch (m_options.exportType & EXP_ANIM_SAMPLE_MASK)
	{
	case EXP_ANIM_SAMPLE_STEP:
		m_animation->m_sampleType = grp::SAMPLE_STEP;
		break;
	case EXP_ANIM_SAMPLE_LINEAR:
		m_animation->m_sampleType = grp::SAMPLE_LINEAR;
		break;
	case EXP_ANIM_SAMPLE_SPLINE:
		m_animation->m_sampleType = grp::SAMPLE_SPLINE;
		break;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
bool CExporter::buildBoneTrack(INode* pNode,
								BoneTrack& boneTrack,
								int iTimeStart, 
								int iTimeEnd,
								int iFrameRate,
								int iFrameTime)
{
	assert(pNode !=  NULL);
	wchar_t unicodeString[256];
	mbstowcs(unicodeString, pNode->GetName(), 255);
	boneTrack.boneName = unicodeString;

	int iFrame = 0;
	for (int iTimeCur = iTimeStart; iTimeCur <= iTimeEnd; iTimeCur += iFrameTime, ++iFrame)
	{
		Matrix3 mNode = pNode->GetNodeTM(iTimeCur);
		Matrix3 mRelative;
		if (pNode->IsRootNode())
		{
			mRelative = mNode;
		}
		else
		{
			Matrix3 mParent = pNode->GetParentTM(iTimeCur);
			if (memcmp(&mNode, &mParent, sizeof(Matrix3)) == 0)
			{
				mRelative.IdentityMatrix();
			}
			else
			{
				mRelative = mNode * Inverse(mParent);
			}
		}
		Vector3 vTranslation;
		Quaternion qRotation;
		Vector3 vScale;
		::GetTransformFromMatrix3(mRelative, vTranslation, qRotation, vScale);

		Vector3Key keyTranslation;
		keyTranslation.time = float(iTimeCur - iTimeStart) / iFrameTime / iFrameRate;
		keyTranslation.transform = vTranslation;
		boneTrack.positionKeys.push_back(keyTranslation);
		
		QuaternionKey keyRotation;
		keyRotation.time = float(iTimeCur - iTimeStart) / iFrameTime / iFrameRate;
		keyRotation.transform = qRotation;
		boneTrack.rotationKeys.push_back(keyRotation);

		if (fabs(vScale.X - 1.0f) > 0.001f
			|| fabs(vScale.Y - 1.0f) > 0.001f
			|| fabs(vScale.Z - 1.0f) > 0.001f)
		{
			Vector3Key keyScale;
			keyScale.time = float(iTimeCur - iTimeStart) / iFrameTime / iFrameRate;
			keyScale.transform = vScale;
			boneTrack.scaleKeys.push_back(keyScale);
		}
	}
	return true;
}

