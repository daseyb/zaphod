#pragma once
#include "baseobject.h"
#include "../SimpleMath.h"

class Sphere :
	public BaseObject
{
private:
	DirectX::BoundingSphere m_Sphere;

public:
	Sphere(float _radius, DirectX::SimpleMath::Vector3 _position);
	~Sphere(void);
	void SetRadius(float _radius);
	void SetPosition(DirectX::SimpleMath::Vector3 _pos) override;
	bool Intersect(const DirectX::SimpleMath::Ray& _ray, Intersection& _intersect) const override;
};

