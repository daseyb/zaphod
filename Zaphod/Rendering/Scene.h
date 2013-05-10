#pragma once
#include "../SimpleMath.h"
#include <vector>
#include <time.h>

class BaseObject;
class Camera;
class Light;

/********************************************
** Scene
** Holds all objects important for rendering.
** Provides an Update method to manipulate 
** those objects over time. Handles ray
** intersection queries.
*********************************************/


class Scene
{
	std::vector<BaseObject*> m_SceneObjects;
	std::vector<Light*> m_SceneLights;

	clock_t m_PrevTime;
	clock_t m_InitTime;

	//Pointer to the current renderer camera
	Camera* m_pCamera;

public:
	Scene(Camera* _cam);
	void Update();
	DirectX::SimpleMath::Color Intersect(const DirectX::SimpleMath::Ray& _ray) const; 
	~Scene(void);
};

