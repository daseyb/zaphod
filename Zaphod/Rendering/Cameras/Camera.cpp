#include "Camera.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;

void Camera::SetPosition(Vector3 _pos)
{
	m_Position = _pos;
}

void Camera::SetRotation(float _yaw, float _pitch, float _roll)
{
	m_Yaw = _yaw;
	m_Pitch = _pitch;
	m_Roll = _roll;

	m_Rotation = Quaternion::CreateFromYawPitchRoll(m_Yaw, m_Pitch, m_Roll);
}

void Camera::LookAt(DirectX::SimpleMath::Vector3 _eye, DirectX::SimpleMath::Vector3 _target, DirectX::SimpleMath::Vector3 _up)
{
	Matrix lookAtMat = Matrix::CreateLookAt(_eye, _target, _up);
	m_Rotation = Quaternion::CreateFromRotationMatrix(lookAtMat);
	m_Position = lookAtMat.Translation();
}

Matrix Camera::GetViewMatrix(void) const
{
	Matrix viewMat = Matrix::CreateFromQuaternion(m_Rotation) * Matrix::CreateTranslation(m_Position);
	viewMat.Invert();
	return viewMat;
}

Camera::~Camera(void)
{
}
