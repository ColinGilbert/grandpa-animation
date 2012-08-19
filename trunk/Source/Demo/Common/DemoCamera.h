#ifndef __DEMO_CAMERA_H__
#define __DEMO_CAMERA_H__

#include "Grandpa.h"

class DemoCamera
{
public:
	enum ProjectionMode
	{
		PERSPECTIVE = 0,
		ORTHO
	};

	enum ControlMode
	{
		MANUAL = 0,
		FIRST_PERSON,
		THIRD_PERSON
	};

	enum CoordinateSystem
	{
		LEFT_HAND = 0,
		RIGHT_HAND
	};

	enum UpAxis
	{
		//does anybody want X_UP?
		Y_UP = 0,
		Z_UP
	};

	enum HotKey
	{
		KEY_UNKNOWN = 0,
		KEY_MOVE_FORWARD,
		KEY_MOVE_BACKWARD,
		KEY_MOVE_LEFT,
		KEY_MOVE_RIGHT,
		KEY_MOVE_UP,
		KEY_MOVE_DOWN,
		KEY_ROTATE_INC_YAW,
		KEY_ROTATE_DEC_YAW,
		KEY_ROTATE_INC_PITCH,
		KEY_ROTATE_DEC_PITCH,
		KEY_ROTATE_INC_ROLL,
		KEY_ROTATE_DEC_ROLL,
		KEY_TOTAL
	};

public:
	DemoCamera();
	~DemoCamera();

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//settings
	void setCoordinateSystem(CoordinateSystem mode);
	CoordinateSystem getCoordinateSystem() const;

	void setUpAxis(UpAxis axis);
	UpAxis getUpAxis() const;

	void setProjectionMode(ProjectionMode mode);
	ProjectionMode getProjectionMode() const;

	void setControlMode(ControlMode mode);
	ControlMode getControlMode() const;

	void setHotKey(HotKey keyType, unsigned int key);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//view functions
	void setViewParams(const grp::Vector3& eyePos, const grp::Vector3& lookAtPos, const grp::Vector3& up);

	void setEyePos(const grp::Vector3& eyePos);			//work only in MANUAL and FIRST_PERSON mode
	void setLookAtPos(const grp::Vector3& lookAtPos);	//work only in MANUAL and THIRD_PERSON mode
	void setUpDirection(const grp::Vector3& up);		//work only in MANUAL mode
	const grp::Vector3& getEyePos() const;
	const grp::Vector3& getLookAtPos() const;
	const grp::Vector3& getUpDirection() const;

	const grp::Vector3& getViewDirection() const;
	const grp::Vector3& getRightDirection() const;

	//euler angles and view distance are only available in FIRST_PERSON and THIRD_PERSON mode
	void setEulerAngle(const grp::Euler& e);
	void setYaw(float yaw);
	void setPitch(float pitch);
	void setRoll(float roll);
	void setViewDistance(float distance, bool animated = true);
	const grp::Euler getEulerAngle() const;
	float getYaw() const;
	float getPitch() const;
	float getRoll() const;
	float getViewDistance() const;

	void setPitchLimit(float minPitch, float maxPitch);

	void setViewDistanceLimit(float minDistance, float maxDistance);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//projection functons
	void setPerspectiveParams(float fov, float aspect, float nearPlane, float farPlane);

	void setOrthoParams(float width, float height, float nearPlane, float farPlane);

	void setNearPlane(float nearPlane);
	void setFarPlane(float farPlane);
	void setFov(float fov);
	void setAspect(float aspect);
	void setWidth(float width);
	void setHeight(float height);
	float getNearPlane() const;
	float getFarPlane() const;
	float getFov() const;
	float getAspect() const;
	float getWidth() const;
	float getHeight() const;

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//utility functions
	void onMouseDown(int mouseX, int mouseY);
	void onMouseUp();
	void onMouseMove(int mouseX, int mouseY);
	void onMouseWheel(int pos);

	void onKeyDown(unsigned int key);
	void onKeyUp(unsigned int key);
	
	void setYawSensitivity(float sensitivity);
	void setPitchSensitivity(float sensitivity);
	void setDistSensitivity(float sensitivity);

	void setDistSpeed(float speed);
	void setMoveSpeed(float speed);
	void setRotateSpeed(float speed);

	void update(double time, float elapsedTime);

	void startShaking(const grp::Vector3& positionAmplitude, const grp::Euler& rotationAmplitude, float frequency);
	void endShaking();

	const grp::Matrix& getViewMatrix() const;
	const grp::Matrix& getProjectionMatrix() const;

private:
	void updateHotKeys(float elapsedTime);

	void updateViewMatrix(float elapsedTime);
	void updateProjectionMatrix();

	HotKey findKey(unsigned int key);

	void buildAnimation();

private:
	ProjectionMode	m_projectionMode;
	ControlMode		m_controlMode;

	grp::Vector3	m_eyePos;
	grp::Vector3	m_lookAtPos;
	grp::Vector3	m_viewDir;
	grp::Vector3	m_upDir;
	grp::Vector3	m_rightDir;

	float			m_width;
	float			m_height;
	float			m_nearPlane;
	float			m_farPlane;
	float			m_fov;
	float			m_aspect;

	grp::Euler		m_eulerAngle;
	float			m_minPitch;
	float			m_maxPitch;
	float			m_viewDistance;
	float			m_destViewDistance;
	float			m_minViewDistance;
	float			m_maxViewDistance;

	bool			m_buttonPushed;
	int				m_lastMouseX;
	int				m_lastMouseY;
	int				m_lastWheelPos;
	float			m_yawSensitivity;
	float			m_pitchSensitivity;
	float			m_distSensitivity;

	float			m_distSpeed;
	float			m_moveSpeed;
	float			m_rotateSpeed;

	grp::Matrix		m_viewMatrix;
	grp::Matrix		m_projectionMatrix;

	CoordinateSystem	m_coordMode;
	UpAxis				m_upAxis;

	unsigned int	m_keys[KEY_TOTAL];
	bool			m_keyState[KEY_TOTAL];

	enum ShakingState
	{
		NOT_SHAKING = 0,
		SHAKING,
		SHAKING_ENDING
	};

	ShakingState				m_shakingState;
	float						m_shakingTime;
	grp::Vector3				m_positionAmplitude;
	grp::Euler					m_rotationAmplitude;
	float						m_shakingFrequency;
	grp::ISpline<grp::Vector3>*	m_positionAnimation;
	grp::ISpline<grp::Euler>*	m_rotationAnimation;

	grp::Euler		m_finalEulerAngle;
	grp::Vector3	m_finalEyePos;

	//记录当前震动方向，停止震动或调整参数时用来平滑
	grp::Vector3	m_translateDirection;
	grp::Euler		m_rotateDirection;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setViewParams(const grp::Vector3& eyePos, const grp::Vector3& lookAtPos, const grp::Vector3& up)
{
	setEyePos(eyePos);
	setLookAtPos(lookAtPos);
	setUpDirection(up);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setPerspectiveParams(float fov, float aspect, float nearPlane, float farPlane)
{
	setProjectionMode(PERSPECTIVE);
	setFov(fov);
	setAspect(aspect);
	setNearPlane(nearPlane);
	setFarPlane(farPlane);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setOrthoParams(float width, float height, float nearPlane, float farPlane)
{
	setProjectionMode(ORTHO);
	setWidth(width);
	setHeight(height);
	setNearPlane(nearPlane);
	setFarPlane(farPlane);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setProjectionMode(ProjectionMode mode)
{
	m_projectionMode = mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline DemoCamera::ProjectionMode DemoCamera::getProjectionMode() const
{
	return m_projectionMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline DemoCamera::ControlMode DemoCamera::getControlMode() const
{
	return m_controlMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setEyePos(const grp::Vector3& eyePos)
{
	m_eyePos = eyePos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setLookAtPos(const grp::Vector3& lookAtPos)
{
	m_lookAtPos = lookAtPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setUpDirection(const grp::Vector3& up)
{
	m_upDir = up;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const grp::Vector3& DemoCamera::getEyePos() const
{
	return m_eyePos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const grp::Vector3& DemoCamera::getLookAtPos() const
{
	return m_lookAtPos;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const grp::Vector3& DemoCamera::getUpDirection() const
{
	return m_upDir;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const grp::Vector3& DemoCamera::getViewDirection() const
{
	return m_viewDir;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const grp::Vector3& DemoCamera::getRightDirection() const
{
	return m_rightDir;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setNearPlane(float nearPlane)
{
	m_nearPlane = nearPlane;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setFarPlane(float farPlane)
{
	m_farPlane = farPlane;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setFov(float fov)
{
	m_fov = fov;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setAspect(float aspect)
{
	m_aspect = aspect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setWidth(float width)
{
	m_width = width;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setHeight(float height)
{
	m_height = height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoCamera::getNearPlane() const
{
	return m_nearPlane;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoCamera::getFarPlane() const
{
	return m_farPlane;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoCamera::getFov() const
{
	return m_fov;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoCamera::getAspect() const
{
	return m_aspect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoCamera::getWidth() const
{
	return m_width;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoCamera::getHeight() const
{
	return m_height;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setEulerAngle(const grp::Euler& e)
{
	m_eulerAngle = e;
	if (m_eulerAngle.pitch > m_maxPitch)
	{
		m_eulerAngle.pitch = m_maxPitch;
	}
	if (m_eulerAngle.pitch < m_minPitch)
	{
		m_eulerAngle.pitch = m_minPitch;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setYaw(float yaw)
{
	m_eulerAngle.yaw = yaw;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setPitch(float pitch)
{
	if (pitch > m_maxPitch)
	{
		pitch = m_maxPitch;
	}
	if (pitch < m_minPitch)
	{
		pitch = m_minPitch;
	}
	m_eulerAngle.pitch = pitch;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setRoll(float roll)
{
	m_eulerAngle.roll = roll;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setViewDistance(float distance, bool animated)
{
	if (distance < m_minViewDistance)
	{
		distance = m_minViewDistance;
	}
	if (m_maxViewDistance > 0.0f && distance > m_maxViewDistance)
	{
		distance = m_maxViewDistance;
	}
	m_destViewDistance = distance;
	if (!animated)
	{
		m_viewDistance = distance;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const grp::Euler DemoCamera::getEulerAngle() const
{
	return m_eulerAngle;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoCamera::getYaw() const
{
	return m_eulerAngle.yaw;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoCamera::getPitch() const
{
	return m_eulerAngle.pitch;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoCamera::getRoll() const
{
	return m_eulerAngle.roll;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float DemoCamera::getViewDistance() const
{
	return m_viewDistance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setPitchLimit(float minPitch, float maxPitch)
{
	m_minPitch = minPitch;
	m_maxPitch = maxPitch;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setViewDistanceLimit(float minDistance, float maxDistance)
{
	m_minViewDistance = minDistance;
	m_maxViewDistance = maxDistance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::onMouseDown(int mouseX, int mouseY)
{
	m_buttonPushed = true;
	m_lastMouseX = mouseX;
	m_lastMouseY = mouseY;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::onMouseUp()
{
	m_buttonPushed = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::onMouseWheel(int pos)
{
	setViewDistance(m_destViewDistance - pos * m_distSensitivity);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setYawSensitivity(float sensitivity)
{
	m_yawSensitivity = sensitivity;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setPitchSensitivity(float sensitivity)
{
	m_pitchSensitivity = sensitivity;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setDistSensitivity(float sensitivity)
{
	m_distSensitivity = sensitivity;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setDistSpeed(float speed)
{
	m_distSpeed = speed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setMoveSpeed(float speed)
{
	m_moveSpeed = speed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setRotateSpeed(float speed)
{
	m_rotateSpeed = speed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const grp::Matrix& DemoCamera::getViewMatrix() const
{
	return m_viewMatrix;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const grp::Matrix& DemoCamera::getProjectionMatrix() const
{
	return m_projectionMatrix;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setCoordinateSystem(CoordinateSystem mode)
{
	m_coordMode = mode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline DemoCamera::CoordinateSystem DemoCamera::getCoordinateSystem() const
{
	return m_coordMode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setUpAxis(UpAxis axis)
{
	m_upAxis = axis;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline DemoCamera::UpAxis DemoCamera::getUpAxis() const
{
	return m_upAxis;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void DemoCamera::setHotKey(HotKey keyType, unsigned int key)
{
	m_keys[keyType] = key;
}

#endif
