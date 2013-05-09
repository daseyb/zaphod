#pragma once
#include "Light.h"
#include "SimpleMath.h"

class DirectionalLight : 
	public Light
{
	DirectX::SimpleMath::Vector3 m_Direction;

public:
	DirectionalLight(DirectX::SimpleMath::Color _col, float _intensity, DirectX::SimpleMath::Vector3 _dir );
	~DirectionalLight(void);
	float GetRange() override;
	DirectX::SimpleMath::Vector3 GetDirection(DirectX::SimpleMath::Vector3 _pos) override;
	float GetDistance(DirectX::SimpleMath::Vector3 _pos) override;
};

