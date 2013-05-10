#pragma once
#include "SimpleMath.h"

/********************************************
** Light
** Base class for all lights. All lights have
** a color and an intensity. Provides virtual
** functions to implement specific behaviour.
*********************************************/

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

