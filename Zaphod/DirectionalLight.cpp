#include "DirectionalLight.h"

using namespace DirectX::SimpleMath;

DirectionalLight::DirectionalLight(Color _col, float _intensity, Vector3 _dir) : Light(_col, _intensity)
{
	m_Direction = _dir;
	m_Direction.Normalize();
}

float DirectionalLight::GetRange()
{
	return 1;
}

Vector3 DirectionalLight::GetDirection(Vector3 _pos)
{
	return -m_Direction;
}

float DirectionalLight::GetDistance(Vector3 _pos)
{
	return 0;
}

DirectionalLight::~DirectionalLight(void)
{

}
