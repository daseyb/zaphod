#define _USE_MATH_DEFINES
#include "PhysicallyBasedCamera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Ray PhysicallyBasedCamera::GetRay(float _x, float _y, int _w, int _h,
                                  std::default_random_engine &_rnd,
                                  float &weight) const {
  float x = _x;
  float y = _y;
  float fovx = m_FOV;          // Horizontal FOV
  float fovy = fovx * float(_h) / _w; // Vertical FOV

  float halfWidth = float(_w) / 2.0f;
  float halfHeight = float(_h) / 2.0f;

  float alpha = tanf(fovx / 2) * ((x - halfWidth) / halfWidth);
  float beta = tanf(fovy / 2) * ((halfHeight - y) / halfHeight);

  Vector3 pos = Vector3(0, 0, 0);
  Vector3 dir =
      alpha * Vector3(1, 0, 0) + beta * Vector3(0, 1, 0) + Vector3(0, 0, -1);
  dir.Normalize();

  Ray result{pos, dir};

  if (m_FocalDistance > 0 && m_LensRadius > 0) {
    Vector2 lensPos = ConcentricSampleDisk(_rnd) * m_LensRadius;
    float ft = m_FocalDistance / -dir.z;

    Vector3 pFocus = dir * ft;

    result.position = Vector3(lensPos.x, lensPos.y, 0);
    result.direction = pFocus - result.position;
    result.direction.Normalize();
  }

  // Aproximate geometric term for sensor
  weight = std::abs(result.direction.Dot(Vector3(0, 0, 1)));

  Matrix viewMatrix = GetViewMatrix();
  result.position = Vector3::Transform(result.position, viewMatrix);
  result.direction = Vector3::TransformNormal(result.direction, viewMatrix);
	result.direction.Normalize();

  return result;
}
