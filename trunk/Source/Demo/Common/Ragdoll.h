#ifndef __RAGDOLL_H__
#define __RAGDOLL_H__

#include "tokamak.h"
#include "d3dx9.h"
#include <vector>
#include "Grandpa.h"

const s32 JOINTS_PER_DUDE = 9;

struct IDirect3DDevice9;

namespace grp
{
class ISkeleton;
class IBone;
class Matrix;
}

struct BoneData
{
	int				geometryType;
	const wchar_t*	boneName;
	grp::Vector3	size;
	grp::Vector3	offset;
	float			mass;
};

struct JointData
{
	s32 bodyA;
	s32 bodyB;
	s32 type; // 0 = ball joint, 1 = hinge joint
	f32 xAxisAngle;
	f32 lowerLimit;
	f32 upperLimit;
	neBool enableLimit;
	neBool enableTwistLimit;
	f32 twistLimit;
};

enum
{
	BONE_BODY,
	BONE_HEAD,
	BONE_RIGHT_ARM,
	BONE_LEFT_ARM,
	BONE_RIGHT_FOREARM,
	BONE_LEFT_FOREARM,
	BONE_RIGHT_THIGH,
	BONE_LEFT_THIGH,
	BONE_RIGHT_LEG,
	BONE_LEFT_LEG,
	BONE_WEAPON,
	BONE_TOTAL
};

class Ragdoll
{
public:
	Ragdoll(grp::ISkeleton* skeleton);
	~Ragdoll();

	void update(double fTime, float fElapsedTime);
	void updateSkeleton();

	void render(IDirect3DDevice9* device, const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj);

private:
	void createSimulator();
	void createGround();
	void createRadDude();
	void getBones();

private:
	grp::ISkeleton*		m_skeleton;

	neSimulator*		m_simulator;
	neAllocatorDefault	m_allocator;
	neAnimatedBody*		m_ground;
	grp::IBone*			m_bones[BONE_TOTAL];
	grp::Matrix			m_relativeTransform[BONE_TOTAL];
	neRigidBody*		m_rigidBodies[BONE_TOTAL];
};

#endif
