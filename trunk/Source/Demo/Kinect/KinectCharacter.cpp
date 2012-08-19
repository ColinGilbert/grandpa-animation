#include "Grandpa.h"
#include "KinectCharacter.h"

//NUI_SKELETON_POSITION_HIP_CENTER = 0,
//NUI_SKELETON_POSITION_SPINE,
//NUI_SKELETON_POSITION_SHOULDER_CENTER,
//NUI_SKELETON_POSITION_HEAD,
//NUI_SKELETON_POSITION_SHOULDER_LEFT,
//NUI_SKELETON_POSITION_ELBOW_LEFT,
//NUI_SKELETON_POSITION_WRIST_LEFT,
//NUI_SKELETON_POSITION_HAND_LEFT,
//NUI_SKELETON_POSITION_SHOULDER_RIGHT,
//NUI_SKELETON_POSITION_ELBOW_RIGHT,
//NUI_SKELETON_POSITION_WRIST_RIGHT,
//NUI_SKELETON_POSITION_HAND_RIGHT,
//NUI_SKELETON_POSITION_HIP_LEFT,
//NUI_SKELETON_POSITION_KNEE_LEFT,
//NUI_SKELETON_POSITION_ANKLE_LEFT,
//NUI_SKELETON_POSITION_FOOT_LEFT,
//NUI_SKELETON_POSITION_HIP_RIGHT,
//NUI_SKELETON_POSITION_KNEE_RIGHT,
//NUI_SKELETON_POSITION_ANKLE_RIGHT,
//NUI_SKELETON_POSITION_FOOT_RIGHT,
//NUI_SKELETON_POSITION_COUNT

const wchar_t* g_knBoneNames[] = 
{
	L"Bip",
	L"Bip Spine",
	L"Bip Neck",
	L"Bip Neck1",
	L"Bip R UpperArm",
	L"Bip R Forearm",
	L"Bip R Hand",
	L"Bip R Finger1",
	L"Bip L UpperArm",
	L"Bip L Forearm",
	L"Bip L Hand",
	L"Bip L Finger1",
	L"Bip R Thigh",
	L"Bip R Calf",
	L"Bip R Foot",
	L"Bip R Toe0",
	L"Bip L Thigh",
	L"Bip L Calf",
	L"Bip L Foot",
	L"Bip L Toe0",
};

KinectCharacter::KinectCharacter(const wchar_t* modelPath, IDirect3DDevice9* device)
	: DemoCharacter(modelPath, device)
	, m_leftFootIk(NULL)
	, m_rightFootIk(NULL)
	, m_leftHandIk(NULL)
	, m_rightHandIk(NULL)
	, m_kinectControl(false)
{
}

void KinectCharacter::buildIk()
{
	grp::ISkeleton* skeleton = m_model->getSkeleton();

	std::vector<grp::IkBoneData> leftIkData, rightIkData;
	leftIkData.push_back(grp::IkBoneData(skeleton->getBoneByName(L"Bip L Calf"),
											grp::EULER_ZXY,
											grp::Euler(-3.0f, 0.0f, 0.0f),
											grp::Euler(-0.1f, 0.0f, 0.0f)));
	leftIkData.push_back(grp::IkBoneData(skeleton->getBoneByName(L"Bip L Thigh"),
											grp::EULER_ZXY,
											grp::Euler(-3.0f, -3.0f, -3.0f),
											grp::Euler(3.0f, 3.0f, 3.0f)));
	rightIkData.push_back(grp::IkBoneData(skeleton->getBoneByName(L"Bip R Calf"),
											grp::EULER_ZXY,
											grp::Euler(-3.0f, 0.0f, 0.0f),
											grp::Euler(-0.1f, 0.0f, 0.0f)));
	rightIkData.push_back(grp::IkBoneData(skeleton->getBoneByName(L"Bip R Thigh"),
											grp::EULER_ZXY,
											grp::Euler(-3.0f, -3.0f, -3.0f),
											grp::Euler(3.0f, 3.0f, 3.0f)));

	m_leftFootIk = skeleton->addIkSolver(skeleton->getBoneByName(L"Bip L Foot"), &leftIkData[0], leftIkData.size(), 0.001f);
	m_rightFootIk = skeleton->addIkSolver(skeleton->getBoneByName(L"Bip R Foot"), &rightIkData[0], rightIkData.size(), 0.001f);

	leftIkData.clear();
	rightIkData.clear();
	leftIkData.push_back(grp::IkBoneData(skeleton->getBoneByName(L"Bip L Forearm"),
											grp::EULER_ZXY,
											grp::Euler(-3.0f, 0.0f, 0.0f),
											grp::Euler(-0.1f, 0.0f, 0.0f)));
	leftIkData.push_back(grp::IkBoneData(skeleton->getBoneByName(L"Bip L UpperArm"),
											grp::EULER_ZXY,
											grp::Euler(-3.0f, -3.0f, -3.0f),
											grp::Euler(3.0f, 3.0f, 3.0f)));
	rightIkData.push_back(grp::IkBoneData(skeleton->getBoneByName(L"Bip R Forearm"),
											grp::EULER_ZXY,
											grp::Euler(-3.0f, 0.0f, 0.0f),
											grp::Euler(-0.1f, 0.0f, 0.0f)));
	rightIkData.push_back(grp::IkBoneData(skeleton->getBoneByName(L"Bip R UpperArm"),
											grp::EULER_ZXY,
											grp::Euler(-3.0f, -3.0f, -3.0f),
											grp::Euler(3.0f, 3.0f, 3.0f)));

	m_leftHandIk = skeleton->addIkSolver(skeleton->getBoneByName(L"Bip L Hand"), &leftIkData[0], leftIkData.size(), 0.001f, false);
	m_rightHandIk = skeleton->addIkSolver(skeleton->getBoneByName(L"Bip R Hand"), &rightIkData[0], rightIkData.size(), 0.001f, false);
}

void KinectCharacter::enableKinectControl(bool enable)
{
	m_kinectControl = enable;
	m_leftFootIk->enable(enable);
	m_rightFootIk->enable(enable);
	m_leftHandIk->enable(enable);
	m_rightHandIk->enable(enable);
}

void KinectCharacter::onPostUpdate(grp::ISkeleton* skeleton)
{
	if (!m_kinectControl)
	{
		return;
	}
	grp::IBone* bones[NUI_SKELETON_POSITION_COUNT];
	for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
	{
		bones[i] = skeleton->getBoneByName(g_knBoneNames[i]);
	}

	grp::IBone* bip = skeleton->getBoneByName(L"Bip");
	grp::Matrix m = bip->getAbsoluteTransform();
	grp::Vector3 centerPos = m_nuiPos[NUI_SKELETON_POSITION_HIP_CENTER];
	centerPos.Z -= 0.2f;
	m.setTranslation(centerPos);
	bip->setAbsoluteTransform(m);
	bip->updateChildren();
		
	m_leftFootIk->setTarget(m_nuiPos[NUI_SKELETON_POSITION_ANKLE_RIGHT]);
	m_rightFootIk->setTarget(m_nuiPos[NUI_SKELETON_POSITION_ANKLE_LEFT]);
	m_leftHandIk->setTarget(m_nuiPos[NUI_SKELETON_POSITION_WRIST_RIGHT]);
	m_rightHandIk->setTarget(m_nuiPos[NUI_SKELETON_POSITION_WRIST_LEFT]);
}

void KinectCharacter::render(ID3DXEffect* effect, const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj)
{
	DemoCharacter::render(effect, mView, mProj);

	if (m_kinectControl)
	{
		for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i)
		{
			grp::AaBox box(m_nuiPos[i] + grp::Vector3(-0.01f, -0.01f, -0.01f),
						   m_nuiPos[i] + grp::Vector3(0.01f, 0.01f, 0.01f));
			drawBox(box);
		}
	}
}
