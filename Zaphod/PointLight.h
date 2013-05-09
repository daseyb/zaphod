#pragma once
#include "light.h"
class PointLight :
	public Light
{
	DirectX::SimpleMath::Vector3 m_Position;
	float m_Range;
public:
	PointLight(DirectX::SimpleMath::Color _col, float _intensity, DirectX::SimpleMath::Vector3 _pos, float _range);
	~PointLight(void);
	float GetRange() override;
	DirectX::SimpleMath::Vector3 GetDirection(DirectX::SimpleMath::Vector3 _pos) override;
	float GetDistance(DirectX::SimpleMath::Vector3 _pos) override;
};

