#include "Ragdoll.h"
#include "d3d9.h"

using namespace grp;

struct DemoData
{
	neV3 pos;
	neV3 boxSize;
};

DemoData gFloor = {{0.0f,0.0f,-0.25f}, {200.0f,200.0f,0.5f}};

#define RAGDOLL_BOX			0
#define RAGDOLL_SPHERE		1
#define RAGDOLL_CYLINDER	2

const wchar_t* g_boneNames[] = 
{
	L"Bip",
	L"Bip Head",
	L"Bip R UpperArm",
	L"Bip L UpperArm",
	L"Bip R Forearm",
	L"Bip L Forearm",
	L"Bip R Thigh",
	L"Bip L Thigh",
	L"Bip R Calf",
	L"Bip L Calf",
	L"Rweapon"
};

BoneData g_bones[] = 
{
	{RAGDOLL_BOX, L"Bip", Vector3(0.25f, 0.45f, 0.68f), Vector3(0,0,-0.34f), 25.0f},	//body	0.68
	{RAGDOLL_SPHERE, L"Bip Head", Vector3(0.3f, 0.3f, 0.2f), Vector3(0,0,0), 8.0f}, //head

	{RAGDOLL_BOX, L"Bip R UpperArm", Vector3(0.303f, 0.15f, 0.15f), Vector3(-0.1515f,0,0), 8.0f}, //right arm
	{RAGDOLL_BOX, L"Bip L UpperArm", Vector3(0.303f, 0.15f, 0.15f), Vector3(-0.1515f,0,0), 8.0f}, //left arm

	{RAGDOLL_BOX, L"Bip R Forearm", Vector3(0.35f, 0.15f, 0.15f), Vector3(-0.175f,0,0), 8.0f}, //right forearm
	{RAGDOLL_BOX, L"Bip L Forearm", Vector3(0.35f, 0.15f, 0.15f), Vector3(-0.175f,0,0), 8.0f}, //left forearm

	{RAGDOLL_BOX, L"Bip R Thigh", Vector3(0.415f, 0.2f, 0.2f), Vector3(-0.2075f,0,0), 8.0f}, //right thigh
	{RAGDOLL_BOX, L"Bip L Thigh", Vector3(0.415f, 0.2f, 0.2f), Vector3(-0.2075f,0,0), 8.0f}, //left thigh

	{RAGDOLL_BOX, L"Bip R Calf", Vector3(0.5f, 0.2f, 0.2f), Vector3(-0.25f,0,0), 8.0f}, //right leg
	{RAGDOLL_BOX, L"Bip L Calf", Vector3(0.5f, 0.2f, 0.2f), Vector3(-0.25f,0,0), 8.0f}, //right leg
	{RAGDOLL_BOX, L"Rweapon", Vector3(2.0f, 0.3f, 0.1f), Vector3(-0.5f,0,0), 15.0f}	//weapon
};

JointData g_joints[] = 
{
	{BONE_HEAD, BONE_BODY,				 1, 0.0f, -NE_PI / 4.0f, NE_PI / 4.0f, true, false, 0.0f}, //head <-> body
	
	{BONE_RIGHT_ARM, BONE_BODY,			 0, NE_PI, 0.0f, NE_PI / 2.5f, true, true, 0.1f}, //
	{BONE_LEFT_ARM, BONE_BODY,			 0, 0.0f, 0.0f, NE_PI / 2.5f, true, true, 0.1f},

	{BONE_RIGHT_FOREARM, BONE_RIGHT_ARM, 1, NE_PI, 0.0f, NE_PI / 2.0f, true, false},
	{BONE_LEFT_FOREARM, BONE_LEFT_ARM,	 1, 0.0f, 0.0f, NE_PI / 2.0f, true, false},

	{BONE_RIGHT_THIGH, BONE_BODY,		 0, NE_PI * 6.0f / 8.0f, 0.0f, NE_PI / 4.0f, true, true, 0.8f},
	{BONE_LEFT_THIGH, BONE_BODY,		 0, NE_PI * 2.0f / 8.0f, 0.0f, NE_PI / 4.0f, true, true, 0.8f},

	{BONE_RIGHT_LEG, BONE_RIGHT_THIGH,	 1, -NE_PI * 0.5f, -NE_PI / 2.0f, 0.0f, true, false},
	{BONE_LEFT_LEG, BONE_LEFT_THIGH,	 1, -NE_PI * 0.5f, -NE_PI / 2.0f, 0.0f, true, false},
};

Ragdoll::Ragdoll(grp::ISkeleton* skeleton)
	: m_simulator(NULL)
	, m_skeleton(skeleton)
{
	createSimulator();
	createGround();
	createRadDude();
	getBones();
}

Ragdoll::~Ragdoll()
{
	if (m_simulator != NULL)
	{
		neSimulator::DestroySimulator(m_simulator);
		m_simulator = NULL;
	}
}

void Ragdoll::createSimulator()
{
	if (m_simulator)
	{
		neSimulator::DestroySimulator(m_simulator);
		m_simulator = NULL;
	}

	neSimulatorSizeInfo sizeInfo;

	sizeInfo.rigidBodiesCount = BONE_TOTAL;
	sizeInfo.animatedBodiesCount = 1;
	sizeInfo.geometriesCount = BONE_TOTAL + 1;
	sizeInfo.constraintsCount = BONE_TOTAL * JOINTS_PER_DUDE;
	s32 totalBody = BONE_TOTAL + 1;
	sizeInfo.overlappedPairsCount = totalBody * (totalBody - 1) / 2;
	neV3 gravity;
	gravity.Set(0.0f, 0.0f, -9.0f);
	{ //dont need any of these
		sizeInfo.rigidParticleCount = 0;
		//sizeInfo.terrainNodesStartCount = 0;
	}

	m_simulator = neSimulator::CreateSimulator(sizeInfo, &m_allocator, &gravity);
}

void Ragdoll::createGround()
{
	m_ground = m_simulator->CreateAnimatedBody();

	neGeometry* geom = m_ground->AddGeometry();	 

	geom->SetBoxSize(gFloor.boxSize);

	m_ground->UpdateBoundingInfo();

	m_ground->SetPos(gFloor.pos);
}

inline neV3& g2ne(grp::Vector3& v)
{
	return *((neV3*)&v);
}

void Ragdoll::createRadDude()
{
	std::vector<JointData> jointData(JOINTS_PER_DUDE);

	memset(m_rigidBodies, 0, sizeof(m_rigidBodies));

	for (s32 i = 0; i < BONE_TOTAL; i++)
	{
		m_rigidBodies[i] = m_simulator->CreateRigidBody();
		m_rigidBodies[i]->CollideConnected(true);

		neGeometry* geom = m_rigidBodies[i]->AddGeometry();
		neV3 inertiaTensor;
		
		grp::IBone* startBone = m_skeleton->getBoneByName(g_bones[i].boneName);
		if (startBone == NULL)
		{
			return;
		}
		const grp::Matrix& boneTransform = startBone->getAbsoluteTransform();
		grp::Vector3 bonePos = boneTransform.getTranslation();
		grp::Quaternion boneRotation = boneTransform.getRotation();
		Vector3 rotatedOffset = boneTransform.rotateVector3(g_bones[i].offset);
		bonePos -= rotatedOffset;

		switch (g_bones[i].geometryType)
		{
		case RAGDOLL_BOX:
			geom->SetBoxSize(g2ne(g_bones[i].size));
			inertiaTensor = neBoxInertiaTensor(g2ne(g_bones[i].size), g_bones[i].mass);
			break;

		case RAGDOLL_SPHERE:
			geom->SetSphereDiameter(g_bones[i].size.X);
			inertiaTensor = neSphereInertiaTensor(g_bones[i].size.X, g_bones[i].mass);
			break;

		case RAGDOLL_CYLINDER:
			geom->SetCylinder(g_bones[i].size.X, g_bones[i].size.Z);
			inertiaTensor = neCylinderInertiaTensor(g_bones[i].size.X, g_bones[i].size.Z, g_bones[i].mass);
			break;
		}

		m_rigidBodies[i]->UpdateBoundingInfo();
		m_rigidBodies[i]->SetInertiaTensor(inertiaTensor);
		m_rigidBodies[i]->SetMass(g_bones[i].mass);

		m_rigidBodies[i]->SetPos(g2ne(bonePos));

		neQ br;
		br.Set(boneRotation.X, boneRotation.Y, boneRotation.Z, boneRotation.W);

		m_rigidBodies[i]->SetRotation(br);
		m_rigidBodies[i]->SetSleepingParameter(0.5f); //make it easier to sleep
	}

	neJoint* joint;
	neT3 jointFrame;

	for (s32 i = 0; i < JOINTS_PER_DUDE; i++)
	{
		joint = m_simulator->CreateJoint(m_rigidBodies[g_joints[i].bodyA], m_rigidBodies[g_joints[i].bodyB]);
		grp::IBone* bone = m_skeleton->getBoneByName(g_bones[g_joints[i].bodyA].boneName);
		grp::Vector3 pos = bone->getAbsoluteTransform().getTranslation();

		jointFrame.SetIdentity();

		jointFrame.pos = g2ne(pos);

		if (g_joints[i].type == 0)
		{
			joint->SetType(neJoint::NE_JOINT_BALLSOCKET);

			neV3 zAxis;
			zAxis.Set(0.0f, 0.0f, 1.0f);

			neQ q;
			q.Set(g_joints[i].xAxisAngle, zAxis);

			jointFrame.rot = q.BuildMatrix3();
		}
		else
		{
			joint->SetType(neJoint::NE_JOINT_HINGE);

			if (i == 3)
			{
				jointFrame.rot[0].Set(1.0f, 0.0f, 0.0f);
				jointFrame.rot[1].Set(0.0f, 1.0f, 0.0f);
				jointFrame.rot[2].Set(0.0f, 0.0f, 1.0f);
			}
			else if (i == 4)
			{
				jointFrame.rot[0].Set(-1.0f, 0.0f, 0.0f);
				jointFrame.rot[1].Set(0.0f, -1.0f, 0.0f);
				jointFrame.rot[2].Set(0.0f, 0.0f, 1.0f);
			}
			else
			{
				jointFrame.rot[0].Set(0.0f, 0.0f, -1.0f);
				jointFrame.rot[1].Set(-1.0f, 0.0f, 0.0f);
				jointFrame.rot[2].Set(0.0f, 1.0f, 0.0f);
			}
		}

		joint->SetJointFrameWorld(jointFrame);
		
		joint->SetLowerLimit(g_joints[i].lowerLimit);

		joint->SetUpperLimit(g_joints[i].upperLimit);

		if (g_joints[i].enableLimit)
			joint->EnableLimit(true);

		if (g_joints[i].enableTwistLimit)
		{
			joint->SetLowerLimit2(g_joints[i].twistLimit);

			joint->EnableLimit2(true);
		}

		joint->SetIteration(4);
		
		joint->Enable(true);
	}
}

void Ragdoll::update( double fTime, float fElapsedTime )
{
	f32 t = 1.0f / 30.0f;
	m_simulator->Advance(t, 1.0f / 60.0f, 1.0f/ 30.0f, NULL);

	//temp
	static float pushTime = 0.1f;
	neV3 force;
	if (pushTime >= 0.0f)
	{
		force.Set(0, 0, 1);
		pushTime -= fElapsedTime;
	}
	else
	{
		force.Set(0, 0, 0);
	}
	m_rigidBodies[0]->SetForce(force);
}

void Ragdoll::updateSkeleton()
{
	for (int i = 0; i < BONE_TOTAL; ++i)
	{
		neRigidBody* body = m_rigidBodies[i];
		if (body == NULL)
		{
			continue;
		}
		neV3 pos = body->GetPos();
		neQ rotation = body->GetRotationQ();
		body->BeginIterateGeometry();
		neGeometry* geom = body->GetNextGeometry();

		grp::Vector3 bonePos(pos.v[0], pos.v[1], pos.v[2]);
		grp::Quaternion q(rotation.X, rotation.Y, rotation.Z, rotation.W);
		grp::Matrix r;
		r.setTranslationRotation(bonePos, q);

		grp::Matrix offsetMatrix(grp::Matrix::IDENTITY);
		offsetMatrix.setTranslation(g_bones[i].offset);
		if (m_bones[i] != NULL)
		{
			m_bones[i]->setAbsoluteTransform(offsetMatrix * r);
			m_bones[i]->updateChildren();
		}
	}
}

void Ragdoll::getBones()
{
	for (int i = 0; i < BONE_TOTAL; ++i)
	{
		m_bones[i] = m_skeleton->getBoneByName(g_boneNames[i]);
	}
}

void Ragdoll::render(IDirect3DDevice9* device, const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj)
{
	device->SetTransform(D3DTS_VIEW, &mView);
	device->SetTransform(D3DTS_PROJECTION, &mProj);

	for (int i = 0; i < BONE_TOTAL; ++i)
	{
		neRigidBody* body = m_rigidBodies[i];
		neV3 pos = body->GetPos();
		neQ rotation = body->GetRotationQ();
		body->BeginIterateGeometry();
		neGeometry* geom = body->GetNextGeometry();
		if (g_bones[i].geometryType == RAGDOLL_BOX)
		{
			neV3 boxSize;
			geom->GetBoxSize(boxSize);
			neV3 nePosition = body->GetPos();
			neQ neRotation = body->GetRotationQ();
			grp::Vector3 grpPosition(nePosition.X(), nePosition.Y(), nePosition.Z());
			grp::Quaternion grpRotation(neRotation.X, neRotation.Y, neRotation.Z, neRotation.W);
			grp::Matrix transform;
			transform.setTranslationRotation(grpPosition, grpRotation);
			D3DXMATRIXA16 mWorld;
			memcpy(&mWorld, &transform, sizeof(D3DXMATRIX));
			device->SetTransform(D3DTS_WORLD, &mWorld);

			std::vector<grp::Vector3> vertices(24);
			grp::AaBox bb;
			bb.MaxEdge = grp::Vector3(boxSize.v[0] / 2, boxSize.v[1] / 2, boxSize.v[2] / 2);
			bb.MinEdge = grp::Vector3(-boxSize.v[0] / 2, -boxSize.v[1] / 2, -boxSize.v[2] / 2);
			vertices[0] = vertices[2] = vertices[4] = bb.MinEdge;
			vertices[1] = vertices[13] = vertices[17] = grp::Vector3(bb.MaxEdge.X, bb.MinEdge.Y, bb.MinEdge.Z);
			vertices[3] = vertices[7] = vertices[9] = grp::Vector3(bb.MinEdge.X, bb.MaxEdge.Y, bb.MinEdge.Z);
			vertices[5] = vertices[11] = vertices[15] = grp::Vector3(bb.MinEdge.X, bb.MinEdge.Y, bb.MaxEdge.Z);
			vertices[18] = vertices[6] = vertices[10] = grp::Vector3(bb.MinEdge.X, bb.MaxEdge.Y, bb.MaxEdge.Z);
			vertices[20] = vertices[14] = vertices[16] = grp::Vector3(bb.MaxEdge.X, bb.MinEdge.Y, bb.MaxEdge.Z);
			vertices[22] = vertices[8] = vertices[12] = grp::Vector3(bb.MaxEdge.X, bb.MaxEdge.Y, bb.MinEdge.Z);
			vertices[19] = vertices[21] = vertices[23] = bb.MaxEdge;

			device->SetFVF(D3DFVF_XYZ);
			device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
			device->DrawPrimitiveUP(D3DPT_LINELIST, 12, &vertices[0], sizeof(grp::Vector3)); 
		}
	}
}
