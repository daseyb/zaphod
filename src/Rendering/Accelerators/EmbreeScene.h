#pragma once
#include <embree2/rtcore.h>
#include <embree2/rtcore_ray.h>
#include "../../SimpleMath.h"

class RenderObject;
struct Intersection;

class EmbreeScene {
private:
  RTCDevice m_Device;
  RTCScene m_Scene;

public:
  EmbreeScene();
  void Clear();
  bool AddObject(RenderObject* obj);
  void CommitScene();
  bool Trace(const DirectX::SimpleMath::Ray &_ray, Intersection &minIntersect) const;
  ~EmbreeScene();
};

