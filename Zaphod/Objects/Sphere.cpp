#include "Sphere.h"
#include "../Intersection.h"

using namespace DirectX::SimpleMath;

Sphere::Sphere(float _radius, Vector3 _position)
{
	m_Sphere = DirectX::BoundingSphere(_position, _radius);
	SetRadius(_radius);
	SetPosition(_position);
}

void Sphere::SetRadius(float _radius)
{
	m_Sphere.Radius = _radius;
}

void Sphere::SetPosition(DirectX::SimpleMath::Vector3 _pos)
{
	BaseObject::SetPosition(_pos);
	m_Sphere.Center = _pos;
}

bool Sphere::Intersect(const Ray& _ray, Intersection& _intersect)
{
	float dist;
	if(_ray.Intersects(m_Sphere, dist))
	{
		if(dist < 0.001f)
			return false;

		_intersect.position = _ray.position + dist * _ray.direction;
		_intersect.normal = _intersect.position - Vector3(m_Sphere.Center.x, m_Sphere.Center.y, m_Sphere.Center.z);
		_intersect.normal.Normalize();
		_intersect.material = m_Material;
		return true;
	}
	return false;
}


Sphere::~Sphere(void)
{
}
