#pragma once
#include "../SimpleMath.h"
#include <vector>
#include <time.h>
#include <random>

class BaseObject;
class Camera;
class Light;
class LightCache;
struct Intersection;


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
	std::vector<BaseObject*> m_SceneLights;
	std::vector<float> m_LightWeights;
	float m_TotalLightWeight;

	std::discrete_distribution<int> m_SampleDist;

	clock_t m_PrevTime;
	clock_t m_InitTime;

	//Pointer to the current renderer camera
	Camera* m_pCamera;
	LightCache* m_LightCache;
public:
	Scene(Camera* _cam);
	void Update();
	DirectX::SimpleMath::Ray SampleLight(std::default_random_engine& _rnd, BaseObject** _outLight, float& le) const;
	bool Test(DirectX::SimpleMath::Vector3 _p1, DirectX::SimpleMath::Vector3 _p2) const;
	bool Trace(const DirectX::SimpleMath::Ray& _ray, Intersection& minIntersect) const;
	~Scene(void);
};

