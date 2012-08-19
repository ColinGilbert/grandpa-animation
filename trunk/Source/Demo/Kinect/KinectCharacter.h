#ifndef __KINECT_CHARACTER_H__
#define __KINECT_CHARACTER_H__

#include "DemoCharacter.h"
#include "MSR_NuiApi.h"

class KinectCharacter : public DemoCharacter
{
public:
	KinectCharacter(const wchar_t* modelPath, IDirect3DDevice9* device);

	void buildIk();

	void enableKinectControl(bool enable);

	void setKinectJointPos(int index, const grp::Vector3& pos);

	virtual void render(ID3DXEffect* effect, const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj);

	virtual void onPostUpdate(grp::ISkeleton* skeleton);

private:
	grp::Vector3 m_nuiPos[NUI_SKELETON_POSITION_COUNT];
	grp::IIkSolver*		m_leftFootIk;
	grp::IIkSolver*		m_rightFootIk;
	grp::IIkSolver*		m_leftHandIk;
	grp::IIkSolver*		m_rightHandIk;
	bool				m_kinectControl;
};

inline void KinectCharacter::setKinectJointPos(int index, const grp::Vector3& pos)
{
	m_nuiPos[index] = pos;
}

#endif
