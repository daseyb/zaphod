#pragma once
#include "../../SimpleMath.h"
#include <random>
#include <memory>
#include <exception>

class Scene;

class Integrator {
protected:
  Scene *m_Scene;

public:
  Integrator(Scene *scene) : m_Scene(scene) {}
  virtual DirectX::SimpleMath::Color
  Intersect(const DirectX::SimpleMath::Ray &_ray, int _depth, bool _isSecondary,
            std::default_random_engine &_rnd) const = 0;
};
