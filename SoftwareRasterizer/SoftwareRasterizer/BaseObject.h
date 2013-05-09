#pragma once

#include "SimpleMath.h"
struct Intersection;

class BaseObject
{
protected:
	DirectX::SimpleMath::Vector3 m_Scale;
	DirectX::SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Quaternion m_Rotation;
	DirectX::SimpleMath::Color m_Diffuse;

public: 
	BaseObject(void);
	~BaseObject(void);
	
	DirectX::SimpleMath::Matrix GetTransform();
	virtual void SetPosition(DirectX::SimpleMath::Vector3 _pos);
	void SetDiffuse(DirectX::SimpleMath::Color _col);

	virtual bool Intersect(const DirectX::SimpleMath::Ray& _ray, Intersection& _intersect) = 0;
};

