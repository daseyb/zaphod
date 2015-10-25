#define _USE_MATH_DEFINES
#include "PhysicallyBasedCamera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Ray PhysicallyBasedCamera::GetRay(int _x, int _y, int _w, int _h,
                                  std::default_random_engine &_rnd,
                                  float &weight) const {
  std::uniform_real_distribution<float> dist(-1, 1);

  float x = _x + dist(_rnd);
  float y = _y + dist(_rnd);
  float fovx =  m_FOV; // Horizontal FOV
  float fovy = fovx * _h / _w; // Vertical FOV

  float halfWidth = _w / 2;
  float halfHeight = _h / 2;

  float alpha = tanf(fovx / 2) * ((x - halfWidth) / halfWidth);
  float beta = tanf(fovy / 2) * ((halfHeight - y) / halfHeight);

  Vector3 pos = Vector3(0, 0, 0);
  Vector3 dir =
      alpha * Vector3(1, 0, 0) + beta * Vector3(0, 1, 0) + Vector3(0, 0, -1);
  dir.Normalize();

  Vector2 lensPos = ConcentricSampleDisk(_rnd) * m_LensRadius;
  float ft = m_FocalDistance / -dir.z;

  Vector3 pFocus = dir * ft;

  Ray result;

  result.position = Vector3(lensPos.x, lensPos.y, 0);
  result.direction = pFocus - result.position;
  result.direction.Normalize();

  Matrix viewMatrix = GetViewMatrix();
  result.position = Vector3::Transform(result.position, viewMatrix);
  result.direction = Vector3::TransformNormal(result.direction, viewMatrix);

  weight = 1;
  return result;
}
