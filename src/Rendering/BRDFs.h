#pragma once
#include "../SimpleMath.h"
#include "Scene.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

struct BRDFSample {
  Vector3 Direction;
  float PDF;
};

inline Vector3 HemisphereSample(float theta, float phi, Vector3 n) {

  float xs = sinf(theta) * cosf(phi);
  float ys = cosf(theta);
  float zs = sinf(theta) * sinf(phi);

  Vector3 y(n.x, n.y, n.z);
  Vector3 h = y;

  if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z))
    h.x = 1.0;
  else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z))
    h.y = 1.0;
  else
    h.z = 1.0;

  Vector3 x = (h.Cross(y));
  x.Normalize();
  Vector3 z = (x.Cross(y));
  z.Normalize();

  Vector3 direction = xs * x + ys * y + zs * z;
  direction.Normalize();
  return direction;
}

inline Vector2 UniformSampleDisk(std::default_random_engine &_rnd) {
  std::uniform_real_distribution<float> dist =
      std::uniform_real_distribution<float>(0, 1);

  float r = sqrtf(dist(_rnd));
  float theta = 2.0f * XM_PI * dist(_rnd);

  return {r * cosf(theta), r * sinf(theta)};
}

inline Vector2 ConcentricSampleDisk(std::default_random_engine &_rnd) {
  std::uniform_real_distribution<float> dist =
      std::uniform_real_distribution<float>(-1, 1);

  float r, theta;
  float sx = dist(_rnd);
  float sy = dist(_rnd);

  if (sx == 0.0f && sy == 0.0f) {
    return {0, 0};
  }

  if (sx >= -sy) {
    if (sx > sy) {
      r = sx;
      if (sy > 0.0f)
        theta = sy / r;
      else
        theta = 8.0f + sy / r;
    } else {
      r = sy;
      theta = 2.0f - sx / r;
    }
  } else {
    if (sx <= sy) {
      r = -sx;
      theta = 4.0f - sy / r;
    } else {
      r = -sy;
      theta = 6.0f + sx / r;
    }
  }

  theta *= XM_PI / 4.0f;
  return {r * cosf(theta), r * sinf(theta)};
}

inline Vector3
CosWeightedRandomHemisphereDirection2(Vector3 n,
                                      std::default_random_engine &_rnd) {
  std::uniform_real_distribution<float> dist =
      std::uniform_real_distribution<float>(0, 1);

  float Xi1 = (float)dist(_rnd);
  float Xi2 = (float)dist(_rnd);


  float u1 = Xi1;
  float phi = Xi2 * 2.0f * XM_PI;

  float f = sqrt(1 - u1);

  float x = f * cos(phi);
  float y = f * sin(phi);
  float z = sqrt(u1);

  Vector3 xDir = abs(n.x) < abs(n.y) ? Vector3(1, 0, 0) : Vector3(0, 1, 0);
  Vector3 yDir = n.Cross(xDir);
  yDir.Normalize();
  xDir = yDir.Cross(n);
  return xDir * x + yDir * y + z * n;
}

inline Vector3
UniformHemisphereSample(Vector3 n,
    std::default_random_engine &_rnd) {
    std::uniform_real_distribution<float> dist =
        std::uniform_real_distribution<float>(0, 1);

    float Xi1 = (float)dist(_rnd);
    float Xi2 = (float)dist(_rnd);

    float theta = Xi1;
    float phi = 2.0f * XM_PI * Xi2;

    float f = sqrt(1 - theta * theta);

    float x = f * cos(phi);
    float y = f * sin(phi);
    float z = theta;

    Vector3 dir = Vector3(x, y, z);
    dir *= dir.Dot(n) < 0.0f ? -1.0f : 1.0f;
    return dir;
}

inline BRDFSample BRDFDiffuse(Vector3 normal, Vector3 view,
                              std::default_random_engine &_rnd) {
  auto out = CosWeightedRandomHemisphereDirection2(normal, _rnd);
  return {out, 1.0f};
  /*auto out = UniformHemisphereSample(normal, _rnd);
  return {out, 2.0f * out.Dot(normal)};*/
}

inline float pow5(float val) {
    return val * val * val * val * val;
}

inline float FresnelSchlick(Vector3 H, Vector3 norm, float n1) {
    float r0 = n1 * n1;
    return r0 + (1 - r0)*pow5(1 - H.Dot(norm));
}

inline float sign(float val) {
    return val < 0 ? -1 : 1;
}

inline BRDFSample BRDFPhong(Vector3 normal, Vector3 view, float kd, float ks, float kt,
                            float roughness, std::default_random_engine &_rnd) {
  std::uniform_real_distribution<float> dist =
      std::uniform_real_distribution<float>(0, 1);

  float total = kd + ks + kt;
  kd /= total;
  ks /= total;
  kt /= total;

  float u = dist(_rnd);
  if (u < kd) {
    return BRDFDiffuse(normal, view, _rnd);
  }


  Vector3 w1, ref;
  if (u < kd + ks) {
      ref = Vector3::Reflect(-view, normal);
  } else {
      float inside = sign(view.Dot(normal));
      float eta = inside < 0 ? 1.0 / 1.5 : 1.5;
      ref = Vector3::Refract(view, -inside * normal, eta);
      if (ref.LengthSquared() < 0.5f) {
          ref = Vector3::Reflect(-view, normal);
      }
  }

  if (roughness == 0) {
    w1 = ref;
  } else {
    float n = 1.0f / roughness;

    float u1 = dist(_rnd), u2 = dist(_rnd);
    float theta = acosf(pow(u1, 1.0f / (n + 1)));
    float phi = 2 * XM_PI * u2;

    w1 = HemisphereSample(theta, phi, ref);
  }

  return {w1, 1};
}