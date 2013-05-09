#pragma once
#include "../SimpleMath.h"
#include <vector>
#include <time.h>

class BaseObject;
class Camera;
class Light;

class Scene
{
	std::vector<BaseObject*> m_SceneObjects;
	std::vector<Light*> m_SceneLights;

	clock_t m_PrevTime;
	clock_t m_InitTime;
	Camera* m_pCamera;

public:
	Scene(Camera* _cam);
	void Update();
	DirectX::SimpleMath::Color Intersect(const DirectX::SimpleMath::Ray& _ray); 
	~Scene(void);
};

