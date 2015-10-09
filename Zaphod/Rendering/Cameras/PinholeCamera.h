#pragma once
#include "Camera.h"


class PinholeCamera : public Camera
{
protected:
	float m_FOV;
public:
	PinholeCamera(float _fov) : m_FOV(_fov) { };
	virtual DirectX::SimpleMath::Ray GetRay(int _x, int _y, int _w, int _h, std::default_random_engine & _rnd, float& weight) const override;
};

