#pragma once
#include "../SimpleMath.h"
#include <vector>
#include <time.h>

class BaseObject;
class Camera;
class Scene
{
	std::vector<BaseObject*> sceneObjects;
	clock_t prevTime;
	clock_t initTime;
	Camera* m_pCamera;

public:
	Scene(Camera* _cam);
	void Update();
	DirectX::SimpleMath::Color Intersect(const DirectX::SimpleMath::Ray& _ray); 
	~Scene(void);
};

