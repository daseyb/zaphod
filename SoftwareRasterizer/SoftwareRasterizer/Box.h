#pragma once
#include "baseobject.h"
#include "SimpleMath.h"

class Box :
	public BaseObject
{
private:
	DirectX::BoundingBox m_Box;
public:
	Box(DirectX::SimpleMath::Vector3 _pos, float _extendX, float _extendY, float _extendZ);
	~Box(void);
	void SetExtendX(float _x);
	void SetExtendY(float _y);
	void SetExtendZ(float _z);
	void SetPosition(DirectX::SimpleMath::Vector3 _pos) override;
	bool Intersect(const DirectX::SimpleMath::Ray& _ray, Intersection& _intersect) override;
};

