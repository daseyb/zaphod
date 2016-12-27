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

  enum class ShiftResult {
    Invertible,
    NotInvertible,
    NotSymmetric
  };

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
  GradientDomainPathTracer::ShiftResult GradientDomainPathTracer::OffsetPath(const Path& base, const Ray &startRay, int length, Path& offset, int& shiftLength, float& weight, float& jacobian) const;
  DirectX::SimpleMath::Color EvaluatePath(const Path & path, int length) const;
};
