#include "PointLight.h"

using namespace DirectX::SimpleMath;

PointLight::PointLight(Color _col, float _intensity, Vector3 _pos, float _range) : Light(_col, _intensity)
{
	m_Position = _pos;
	m_Range = _range;
}

float PointLight::GetRange()
{
	return m_Range;
}

Vector3 PointLight::GetDirection(Vector3 _pos)
{
	Vector3 dir = _pos - m_Position;
	dir.Normalize();
	return -dir;
}

float PointLight::GetDistance(Vector3 _pos)
{
	return (_pos - m_Position).Length();
}


PointLight::~PointLight(void)
{
}
