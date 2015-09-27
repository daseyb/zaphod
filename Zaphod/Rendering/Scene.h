#pragma once
#include "../SimpleMath.h"
#include <vector>
#include <time.h>
#include <random>

class BaseObject;
class Camera;
class Light;
class LightCache;

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
  std::vector<float> m_LightIntervals;

  std::default_random_engine m_Rnd;
  std::piecewise_constant_distribution<double> m_SampleDist;

	clock_t m_PrevTime;
	clock_t m_InitTime;

	//Pointer to the current renderer camera
	Camera* m_pCamera;
	LightCache* m_LightCache;
public:
	Scene(Camera* _cam);
	void Update();
  DirectX::SimpleMath::Ray SampleLight(std::default_random_engine _rnd) const;
	DirectX::SimpleMath::Color Intersect(const DirectX::SimpleMath::Ray& _ray, int _depth, bool _isSecondary) const;
	~Scene(void);
};

