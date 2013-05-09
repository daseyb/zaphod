#pragma once
#include "SimpleMath.h"

class Light
{
	DirectX::SimpleMath::Color m_Color;
	float m_Intensity;
public:
	Light(DirectX::SimpleMath::Color _col, float _intensity);
	~Light(void);
	DirectX::SimpleMath::Color GetColor();
	virtual float GetRange() = 0;
	virtual DirectX::SimpleMath::Vector3 GetDirection(DirectX::SimpleMath::Vector3 _pos) = 0;
	virtual float GetDistance(DirectX::SimpleMath::Vector3 _pos) = 0;
};

