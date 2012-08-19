#include "DemoCamera.h"
#include "math.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
DemoCamera::DemoCamera()
	: m_projectionMode(PERSPECTIVE)
	, m_controlMode(MANUAL)
	, m_eyePos(1, 0, 0)
	, m_lookAtPos(0, 0, 0)
	, m_upDir(0, 1, 0)
	, m_width(4.0f)
	, m_height(3.0f)
	, m_nearPlane(0.1f)
	, m_farPlane(10.0f)
	, m_fov(3.1416f / 4)
	, m_aspect(4.0f / 3.0f)
	, m_eulerAngle(0.0f, 0.0f, 0.0f)
	, m_minPitch(-1.5f)
	, m_maxPitch(1.5f)
	, m_minViewDistance(0.0f)
	, m_maxViewDistance(0.0f)	//0: no limit
	, m_buttonPushed(false)
	, m_lastMouseX(0)
	, m_lastMouseY(0)
	, m_lastWheelPos(0)
	, m_yawSensitivity(1.0f / 200)
	, m_pitchSensitivity(1.0f / 200)
	, m_viewDistance(2.0f)
	, m_destViewDistance(2.0f)
	, m_distSensitivity(1.0f / 400)
	, m_distSpeed(5.0f)
	, m_moveSpeed(2.0f)
	, m_rotateSpeed(1.5f)
	, m_coordMode(LEFT_HAND)
	, m_upAxis(Y_UP)
	, m_shakingState(NOT_SHAKING)
	, m_positionAnimation(NULL)
	, m_rotationAnimation(NULL)
{
	memset(m_keys, 0, sizeof(m_keys));
	memset(m_keyState, 0, sizeof(m_keyState));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DemoCamera::~DemoCamera()
{
	if (m_positionAnimation != NULL)
	{
		grp::destroySpline(m_positionAnimation);
	}
	if (m_rotationAnimation != NULL)
	{
		grp::destroySpline(m_rotationAnimation);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::setControlMode(ControlMode mode)
{
	if (mode == m_controlMode)
	{
		return;
	}
	//do some conversions...
	m_controlMode = mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::update(double time, float elapsedTime)
{
	updateHotKeys(elapsedTime);

	//view distance animation
	if (m_viewDistance != m_destViewDistance)
	{
		float moveDist = elapsedTime * m_distSpeed;
		if (moveDist > fabs(m_destViewDistance - m_viewDistance))
		{
			m_viewDistance = m_destViewDistance;
		}
		else
		{
			m_viewDistance += (m_destViewDistance > m_viewDistance ? moveDist : -moveDist);
		}
	}
	updateViewMatrix(elapsedTime);
	//not necessary to update every time but hey, how many cameras can you have at the same time?
	updateProjectionMatrix();
}

const int CYCLE_COUNT = 10;

///////////////////////////////////////////////////////////////////////////////////////////////////
float randInRange(float range)
{
	return range * ((float)rand() / RAND_MAX - 0.5f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::startShaking(const grp::Vector3& positionAmplitude,
							  const grp::Euler& rotationAmplitude,
							  float frequency)
{
	m_positionAmplitude = positionAmplitude;
	m_rotationAmplitude = rotationAmplitude;
	m_shakingFrequency = frequency;

	if (m_shakingState == NOT_SHAKING)
	{
		m_finalEulerAngle = m_eulerAngle;
		m_finalEyePos = m_eyePos;
		m_rotateDirection = grp::Euler(0,0,0);
		m_translateDirection = grp::Vector3(0,0,0);
	}
	buildAnimation();
	m_shakingState = SHAKING;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::endShaking()
{
	if (m_shakingState == NOT_SHAKING)
	{
		return;
	}
	m_shakingState = SHAKING_ENDING;
	
	m_positionAnimation->clear();
	m_rotationAnimation->clear();
	
	float cycle = m_shakingFrequency == 0.0f ? 0.2f : 1.0f / m_shakingFrequency;
	float time = 0.0f;
	grp::Vector3 posKey = m_finalEyePos - m_eyePos;
	grp::Euler dirKey = m_finalEulerAngle - m_eulerAngle;

	m_positionAnimation->addKey(posKey, 0.0f);
	m_rotationAnimation->addKey(dirKey, 0.0f);

	grp::Vector3 posOffset = m_translateDirection * cycle * 0.5f;
	grp::Euler dirOffset = m_rotateDirection * cycle * 0.5f;
	
	time += (cycle * 0.3f);
	posKey += (m_translateDirection * cycle * 0.3f);
	dirKey += (m_rotateDirection * cycle * 0.3f);
	m_positionAnimation->addKey(posKey, time);
	m_rotationAnimation->addKey(dirKey, time);
	
	time += (cycle * 0.2f);
	posKey += (m_translateDirection * cycle * 0.2f);
	dirKey += (m_rotateDirection * cycle * 0.2f);
	m_positionAnimation->addKey(posKey, time);
	m_rotationAnimation->addKey(dirKey, time);

	time += cycle;
	m_positionAnimation->addKey(grp::Vector3(0,0,0), time);
	m_rotationAnimation->addKey(grp::Euler(0,0,0), time);

	m_positionAnimation->buildKnots();
	m_rotationAnimation->buildKnots();

	m_shakingTime = 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::updateViewMatrix(float elapsedTime)
{
	if (m_controlMode == MANUAL)
	{
		m_viewDir = m_lookAtPos - m_eyePos;
		m_viewDir.normalize();
	}
	else
	{
		grp::Quaternion q;
		if (m_upAxis == Y_UP)
		{
			q.setEuler_yxz(m_eulerAngle);
		}
		else
		{
			q.setEuler_zxy(m_eulerAngle);
		}
		grp::Vector3 originViewDir = (m_upAxis == Y_UP)?
									grp::Vector3(0, 0, 1):
									grp::Vector3(0, 1, 0);
		m_viewDir = -(q * originViewDir);
		grp::Vector3 originUpDir = (m_upAxis == Y_UP)?
									grp::Vector3(0, 1, 0):
									grp::Vector3(0, 0, 1);
		m_upDir = q * originUpDir;

		if (m_controlMode == FIRST_PERSON)
		{
			m_lookAtPos = m_eyePos + m_viewDir;
		}
		else if (m_controlMode == THIRD_PERSON)
		{
			m_eyePos = m_lookAtPos - m_viewDir * m_viewDistance;
		}
	}
	
	grp::Vector3 lastEyePos = m_finalEyePos;
	grp::Euler lastRotation = m_finalEulerAngle;

	m_finalEyePos = m_eyePos;
	grp::Vector3 finalLookAtPos = m_lookAtPos;
	if (m_shakingState > NOT_SHAKING && elapsedTime > 0.0f)
	{
		grp::Vector3 positionOffset = m_positionAnimation->sample(m_shakingTime);
		m_finalEyePos += positionOffset;
		grp::Euler rotationOffset = m_rotationAnimation->sample(m_shakingTime);
		m_finalEulerAngle = m_eulerAngle + rotationOffset;

		grp::Quaternion q;
		if (m_upAxis == Y_UP)
		{
			q.setEuler_yxz(m_finalEulerAngle);
		}
		else
		{
			q.setEuler_zxy(m_finalEulerAngle);
		}
		grp::Vector3 originViewDir = (m_upAxis == Y_UP)?
									 grp::Vector3(0, 0, 1):
									 grp::Vector3(0, 1, 0);
		grp::Vector3 viewDir = -(q * originViewDir);
		finalLookAtPos = m_finalEyePos + viewDir;

		m_shakingTime += elapsedTime;

		m_translateDirection = (m_finalEyePos - lastEyePos) / elapsedTime;
		m_rotateDirection = (m_finalEulerAngle - lastRotation) / elapsedTime;
		if (m_shakingTime >= m_positionAnimation->maxTime())
		{
			if (m_shakingState == SHAKING_ENDING)
			{
				grp::destroySpline(m_positionAnimation);
				grp::destroySpline(m_rotationAnimation);
				m_positionAnimation = NULL;
				m_rotationAnimation = NULL;
				m_shakingState = NOT_SHAKING;
			}
			else
			{
				buildAnimation();
			}
		}
	}
	if (m_coordMode == LEFT_HAND)
	{
		m_rightDir = m_upDir.cross(m_viewDir);
		m_viewMatrix.buildLookAtLH(m_finalEyePos, finalLookAtPos, m_upDir);
	}
	else
	{
		m_rightDir = m_viewDir.cross(m_upDir);
		m_viewMatrix.buildLookAtRH(m_finalEyePos, finalLookAtPos, m_upDir);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::updateProjectionMatrix()
{
	if (m_projectionMode == PERSPECTIVE)
	{
		if (m_coordMode == LEFT_HAND)
		{
			m_projectionMatrix.buildProjectPerspectiveFovLH(m_fov, m_aspect, m_nearPlane, m_farPlane);
		}
		else
		{
			m_projectionMatrix.buildProjectPerspectiveFovRH(m_fov, m_aspect, m_nearPlane, m_farPlane);
		}
	}
	else if (m_projectionMode == ORTHO)
	{
		if (m_coordMode == LEFT_HAND)
		{
			m_projectionMatrix.buildProjectOrthoLH(m_width, m_height, m_nearPlane, m_farPlane);
		}
		else
		{
			m_projectionMatrix.buildProjectOrthoRH(m_width, m_height, m_nearPlane, m_farPlane);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::onMouseMove(int mouseX, int mouseY)
{
	if (m_buttonPushed)
	{
		int xOffset = mouseX - m_lastMouseX;
		int yOffset = mouseY - m_lastMouseY;
		if (m_upAxis == Y_UP)
		{
			yOffset = -yOffset;
		}
		else
		{
			xOffset = -xOffset;
		}
		setYaw(m_eulerAngle.yaw + xOffset * m_yawSensitivity);
		setPitch(m_eulerAngle.pitch + yOffset * m_pitchSensitivity);
		m_lastMouseX = mouseX;
		m_lastMouseY = mouseY;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::onKeyDown(unsigned int key)
{
	HotKey keyType = findKey(key);
	if (keyType != KEY_UNKNOWN)
	{
		m_keyState[keyType] = true;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::onKeyUp(unsigned int key)
{
	HotKey keyType = findKey(key);
	if (keyType != KEY_UNKNOWN)
	{
		m_keyState[keyType] = false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DemoCamera::HotKey DemoCamera::findKey(unsigned int key)
{
	for (unsigned int i = 1; i < KEY_TOTAL; ++i)
	{
		if (m_keys[i] == key)
		{
			return (HotKey)i;
		}
	}
	return KEY_UNKNOWN;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::updateHotKeys(float elapsedTime)
{
	//move
	float moveDist = elapsedTime * m_moveSpeed;
	if (m_controlMode == FIRST_PERSON)
	{
		if (m_keyState[KEY_MOVE_FORWARD])
		{
			m_eyePos += (m_viewDir * moveDist);
		}
		if (m_keyState[KEY_MOVE_BACKWARD])
		{
			m_eyePos -= (m_viewDir * moveDist);
		}
		if (m_keyState[KEY_MOVE_LEFT])
		{
			m_eyePos -= (m_rightDir * moveDist);
		}
		if (m_keyState[KEY_MOVE_RIGHT])
		{
			m_eyePos += (m_rightDir * moveDist);
		}
		if (m_keyState[KEY_MOVE_UP])
		{
			m_eyePos += (m_upDir * moveDist);
		}
		if (m_keyState[KEY_MOVE_DOWN])
		{
			m_eyePos -= (m_upDir * moveDist);
		}
	}
	//rotate
	float rotateAngle = elapsedTime * m_rotateSpeed;
	if (m_controlMode == FIRST_PERSON || m_controlMode == THIRD_PERSON)
	{
		if (m_keyState[KEY_ROTATE_INC_YAW])
		{
			m_eulerAngle.yaw += rotateAngle;
		}
		if (m_keyState[KEY_ROTATE_DEC_YAW])
		{
			m_eulerAngle.yaw -= rotateAngle;
		}
		if (m_keyState[KEY_ROTATE_INC_PITCH])
		{
			m_eulerAngle.pitch += rotateAngle;
			if (m_eulerAngle.pitch > m_maxPitch)
			{
				m_eulerAngle.pitch = m_maxPitch;
			}
		}
		if (m_keyState[KEY_ROTATE_DEC_PITCH])
		{
			m_eulerAngle.pitch -= rotateAngle;
			if (m_eulerAngle.pitch < m_minPitch)
			{
				m_eulerAngle.pitch = m_minPitch;
			}
		}
		if (m_keyState[KEY_ROTATE_INC_ROLL])
		{
			m_eulerAngle.roll += rotateAngle;
		}
		if (m_keyState[KEY_ROTATE_DEC_ROLL])
		{
			m_eulerAngle.roll -= rotateAngle;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCamera::buildAnimation()
{
	if (m_positionAnimation != NULL)
	{
		m_positionAnimation->clear();
	}
	else
	{
		m_positionAnimation = grp::createSpline<grp::Vector3>();
	}
	if (m_rotationAnimation != NULL)
	{
		m_rotationAnimation->clear();
	}
	else
	{
		m_rotationAnimation = grp::createSpline<grp::Euler>();
	}
	float cycle = m_shakingFrequency == 0.0f ? 0.2f : 1.0f / m_shakingFrequency;
	float time = 0.0f;
	grp::Vector3 posKey = m_finalEyePos - m_eyePos;
	grp::Euler dirKey = m_finalEulerAngle - m_eulerAngle;

	m_positionAnimation->addKey(posKey, 0.0f);
	m_rotationAnimation->addKey(dirKey, 0.0f);
	
	time += (cycle * 0.3f);
	posKey += (m_translateDirection * cycle * 0.3f);
	dirKey += (m_rotateDirection * cycle * 0.3f);
	m_positionAnimation->addKey(posKey, time);
	m_rotationAnimation->addKey(dirKey, time);
	
	time += (cycle * 0.2f);
	posKey += (m_translateDirection * cycle * 0.2f);
	dirKey += (m_rotateDirection * cycle * 0.2f);
	m_positionAnimation->addKey(posKey, time);
	m_rotationAnimation->addKey(dirKey, time);

	for (int i = 0; i < CYCLE_COUNT; ++i)
	{
		time += cycle;
		m_positionAnimation->addKey(grp::Vector3(randInRange(m_positionAmplitude.X),
												 randInRange(m_positionAmplitude.Y),
												 randInRange(m_positionAmplitude.Z)), time);
		m_rotationAnimation->addKey(grp::Euler(randInRange(m_rotationAmplitude.yaw),
											   randInRange(m_rotationAmplitude.pitch),
											   randInRange(m_rotationAmplitude.roll)), time);
	}
	m_positionAnimation->buildKnots();
	m_rotationAnimation->buildKnots();
	
	m_shakingTime = 0.0f;
}
