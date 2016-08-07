#include "Scene.h"
#include "../Objects/BaseObject.h"
#include "../Objects/Sphere.h"
#include "../Objects/Box.h"
#include "../Objects/Mesh.h"
#include "Cameras/Camera.h"
#include "Accelerators/LightCache.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <math.h>
#include "Materials/Material.h"
#include "Materials/DiffuseMaterial.h"
#include "Materials/SpecularMaterial.h"
#include "Materials/EmissionMaterial.h"

#define USE_LIGHTCACHE 0

using namespace DirectX;
using namespace DirectX::SimpleMath;


Scene::Scene(Camera *_cam, std::vector<BaseObject *>& sceneObjects ) {
  // Initialize object lists
  m_SceneObjects = std::move(sceneObjects);
  m_SceneLights = std::vector<BaseObject *>();

  m_CustomIntersectObjects = std::vector<BaseObject*>();

  m_TotalLightWeight = 0;
  
  for (auto obj : m_SceneObjects) {

    if (!m_EmbreeScene.AddObject(obj)) {
      m_CustomIntersectObjects.push_back(obj);
    }
    
    if (obj->GetMaterial()->IsLight()) {
      m_SceneLights.push_back(obj);
      float weight = obj->CalculateWeight();
      if (weight == 0) {
        continue;
      }
      m_LightWeights.push_back(weight);
      m_TotalLightWeight += weight;
    }
  }

  m_EmbreeScene.CommitScene();

  m_SampleDist = std::discrete_distribution<>(std::begin(m_LightWeights),
                                              std::end(m_LightWeights));

  // Set the start time
  m_InitTime = clock();

  // Set the camera pointer and move the camera to it's start position
  m_pCamera = _cam;

  m_LightCache =
      new LightCache(BoundingBox(Vector3(0, 0, 0), Vector3(20, 20, 20)));
}

void Scene::Update() {
  // Update the time values
  clock_t time = clock();
  double deltaTime = (double)(time - m_PrevTime) / CLOCKS_PER_SEC;
  double totalTime = (double)(time - m_InitTime) / CLOCKS_PER_SEC;

  m_PrevTime = time;
}

Ray Scene::SampleLight(std::default_random_engine &_rnd, BaseObject **_outLight,
                       float &le) const {
  assert(m_SceneLights.size() > 0);
  int lightIndex = (int)m_SampleDist(_rnd);
  *_outLight = m_SceneLights[lightIndex];
  le = m_LightWeights[lightIndex] / m_TotalLightWeight;
  return (*_outLight)->Sample(_rnd);
}

bool Scene::Trace(const DirectX::SimpleMath::Ray &_ray,
                  Intersection &minIntersect) const {
  float minDist = FLT_MAX;
  Intersection intersect;
  bool intersectFound = false;

  if (m_EmbreeScene.Trace(_ray, intersect)) {
    float dist = (intersect.position - _ray.position).LengthSquared();
    minDist = dist;
    intersectFound = true;
    minIntersect = intersect;
  }

  // Find the nearest intersection
  for (auto obj : m_CustomIntersectObjects) {
    if (obj->Intersect(_ray, intersect)) {
      float dist = (intersect.position - _ray.position).LengthSquared();
      if (dist < minDist) {
        minDist = dist;
        intersectFound = true;
        minIntersect = intersect;
        minIntersect.hitObject = obj;
      }
    }
  }

  return intersectFound;
}

Scene::~Scene(void) {
  for (auto obj : m_SceneObjects) {
    delete obj;
  }

  m_SceneObjects.clear();

  m_pCamera = nullptr;
}
