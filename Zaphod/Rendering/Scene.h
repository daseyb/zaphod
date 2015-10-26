#pragma once
#include "../SimpleMath.h"
#include <vector>
#include <time.h>
#include <random>
#include "../Geometry/Intersection.h"
#include "Accelerators/EmbreeScene.h"

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

class Scene {
  std::vector<BaseObject *> m_SceneObjects;
  std::vector<BaseObject *> m_SceneLights;
  std::vector<BaseObject *> m_CustomIntersectObjects;

  std::vector<float> m_LightWeights;
  float m_TotalLightWeight;

  std::discrete_distribution<int> m_SampleDist;

  clock_t m_PrevTime;
  clock_t m_InitTime;

  // Pointer to the current renderer camera
  Camera *m_pCamera;
  LightCache *m_LightCache;

  EmbreeScene m_EmbreeScene;

public:
  Scene(Camera *_cam, std::vector<BaseObject *>& sceneObjects);
  void Update();
  DirectX::SimpleMath::Ray SampleLight(std::default_random_engine &_rnd,
                                       BaseObject **_outLight, float &le) const;
  bool Trace(const DirectX::SimpleMath::Ray &_ray,
             Intersection &minIntersect) const;

  inline bool Test(DirectX::SimpleMath::Vector3 _p1,
                   DirectX::SimpleMath::Vector3 _p2) const {
    auto dir = _p2 - _p1;
    dir.Normalize();

    DirectX::SimpleMath::Ray r = {_p1 + dir * 0.001f, dir};
    Intersection intersect;
    if (!Trace(r, intersect)) {
      return false;
    }
    return DirectX::SimpleMath::Vector3::DistanceSquared(intersect.position,
                                                         _p2) < 0.001f;
  }

  ~Scene(void);
};
