#pragma once
#include "Integrator.h"
#include "../../Geometry/Intersection.h"
#include "../BRDFs.h"
#include "../Materials/Material.h"

#include <array>


class GradientDomainPathTracer : Integrator {
public:
  GradientDomainPathTracer(Scene *scene, Camera* camera) : Integrator(scene, camera) {}
  virtual DirectX::SimpleMath::Color
  Intersect(const DirectX::SimpleMath::Ray &_ray, int _depth, bool _isSecondary,
            std::default_random_engine &_rnd) const override;

  virtual DirectX::SimpleMath::Color Sample(float x, float y, int w, int h, std::default_random_engine& _rnd) const override;
private:

  struct PathVertex {
    enum Type {
      Camera,
      Diffuse,
      Specular,
      Light
    };

    Intersection intersect;
    BRDFSample sample;
    Type type;
  };

  typedef std::array<PathVertex, 15> Path;

  Path TracePath(const DirectX::SimpleMath::Ray & _ray, int _depth, std::default_random_engine & _rnd, int& length) const;
  bool GradientDomainPathTracer::OffsetPath(const Path& base, const Ray &startRay, int length, Path& offset, float& weight, float& jacobian) const;
  DirectX::SimpleMath::Color EvaluatePath(const Path & path, int length) const;
};
