#pragma once
#include "Integrator.h"
#include "../../Geometry/Intersection.h"

struct Material;
class BaseObject;

class BidirectionalPathTracer : public Integrator {
private:
  struct PathVertex {
    DirectX::SimpleMath::Vector3 Pos;
    DirectX::SimpleMath::Vector3 Normal;
    DirectX::SimpleMath::Vector3 In;
    DirectX::SimpleMath::Vector3 Out;

    Intersection Intersect;
    Material *Material;
    BaseObject *Prim;
    float BrdfWeight;
    float RelativeWeight;
  };

  typedef std::vector<PathVertex> Path;

  Path MakePath(const DirectX::SimpleMath::Ray &_startRay, int _depth,
                std::default_random_engine &_rnd) const;
  float G(const PathVertex &v0, const PathVertex &v1) const;
  DirectX::SimpleMath::Color EvalPath(const Path &eye, int nEye,
                                      const Path &light, int nLight) const;

	DirectX::SimpleMath::Color EvalPath(const Path & eye, int nEye) const;

  DirectX::SimpleMath::Color
  IlluminatePoint(DirectX::SimpleMath::Vector3 pos,
                  DirectX::SimpleMath::Vector3 normal,
                  std::default_random_engine &_rnd) const;

public:
  BidirectionalPathTracer(Scene *scene, Camera* camera, int w, int h) : Integrator(scene, camera, w, h) {}
  virtual DirectX::SimpleMath::Color
  Intersect(const DirectX::SimpleMath::Ray &_ray, int _depth, bool _isSecondary,
            std::default_random_engine &_rnd) const override;
};
