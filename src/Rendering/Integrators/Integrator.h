#pragma once
#include "../../SimpleMath.h"
#include <random>
#include <memory>
#include <exception>
#include "../Cameras/Camera.h"

class Scene;

class Integrator {
protected:
  Scene *m_Scene;
  Camera *m_Camera;

public:
  Integrator(Scene *scene, Camera *camera) : m_Scene(scene), m_Camera(camera) {}
  virtual DirectX::SimpleMath::Color
  Intersect(const DirectX::SimpleMath::Ray &_ray, int _depth, bool _isSecondary,
            std::default_random_engine &_rnd) const = 0;

  virtual DirectX::SimpleMath::Color Sample(float x, float y, int w, int h, std::default_random_engine& _rnd) const {
      float weight;
      DirectX::SimpleMath::Ray  ray = m_Camera->GetRay(x, y, w, h, _rnd, weight);

      if (weight > FLT_EPSILON) {
          return Intersect(ray, 8, false, _rnd);
      }
      return{ 0,0,0 };
  }
};
