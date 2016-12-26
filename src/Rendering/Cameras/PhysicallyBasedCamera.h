#pragma once
#include "PinholeCamera.h"
#include "../BRDFs.h"

class PhysicallyBasedCamera : public PinholeCamera {
private:
  float m_FocalDistance, m_LensRadius;

public:
  PhysicallyBasedCamera(float _focalDistance, float _lensRadius, float _fov)
      : m_FocalDistance(_focalDistance), m_LensRadius(_lensRadius),
        PinholeCamera(_fov){};
  virtual DirectX::SimpleMath::Ray GetRay(float _x, float _y, int _w, int _h,
                                          std::default_random_engine &_rnd,
                                          float &weight) const override;
};
