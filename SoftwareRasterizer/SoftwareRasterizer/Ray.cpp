#include "Ray.h"
using namespace DirectX::SimpleMath;

Ray::Ray(Vector3 _start, Vector3 _dir)
{
	m_Start = _start;
	m_Dir = _dir.Normalize();
}

Ray::Ray(const Ray& _other)
{
	m_Start = _other.m_Start;
	m_Dir = _other.m_Dir;
}

XMFLOAT3 Ray::GetStart(void) const
{
	return m_Start;
}

XMFLOAT3 Ray::GetDir(void) const
{
	return m_Dir;
}

XMFLOAT3 Ray::GetPoint(float _t) const
{
	if(_t <= 0)
		return m_Start;

	XMFLOAT3 point;
	XMStoreFloat3(&point, XMLoadFloat3(&m_Start) + _t * XMLoadFloat3(&m_Dir));
	return point;
}

Ray::~Ray(void)
{
}
