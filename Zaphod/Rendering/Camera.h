#pragma once
#include "../SimpleMath.h"

class Camera
{
private:
	DirectX::SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Quaternion m_Rotation;
	float m_Yaw, m_Pitch, m_Roll;

public:
	Camera(void);
	void SetPosition(DirectX::SimpleMath::Vector3 _pos);
	void SetRotation(float _yaw, float _pitch, float _roll);
	void LookAt(DirectX::SimpleMath::Vector3 _eye, DirectX::SimpleMath::Vector3 _target, DirectX::SimpleMath::Vector3 _up);

	DirectX::SimpleMath::Matrix GetViewMatrix(void) const;

	~Camera(void);
};

