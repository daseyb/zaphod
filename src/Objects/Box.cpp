#include "Box.h"
#include "../Geometry/Intersection.h"
#include <iterator>

using namespace DirectX::SimpleMath;

Box::Box(Vector3 _pos, Vector3 _extends, BaseObject* parent) : RenderObject(parent) {
  SetPosition(_pos);
  m_Box.Center = _pos;
  m_Box.Extents = _extends;
}

void Box::SetExtendX(float _x) { m_Box.Extents.x = _x; }

void Box::SetExtendY(float _y) { m_Box.Extents.y = _y; }

void Box::SetExtendZ(float _z) { m_Box.Extents.z = _z; }

void Box::SetPosition(Vector3 _pos) {
  BaseObject::SetPosition(_pos);
  m_Box.Center = _pos;
}

float Box::CalculateWeight() {
  m_SampleWeights[0] = m_Box.Extents.y * m_Box.Extents.z * 8;
  m_SampleWeights[1] = m_Box.Extents.x * m_Box.Extents.z * 8;
  m_SampleWeights[2] = m_Box.Extents.x * m_Box.Extents.y * 8;

  m_SampleDist = std::discrete_distribution<>(
      {m_SampleWeights[0], m_SampleWeights[1], m_SampleWeights[2]});
  m_Weight = m_SampleWeights[0] + m_SampleWeights[1] + m_SampleWeights[2];
  return m_Weight;
}

Ray Box::Sample(std::default_random_engine &rnd) {
  auto axis = m_SampleDist(rnd);
  std::uniform_real_distribution<float> dist(-1, 1);

  float dir = dist(rnd) < 0.0f ? -1.0f : 1.0f;
  Ray result;
  switch (axis) {
  case 0:
    result.position =
        Vector3(m_Box.Extents.x * dir, m_Box.Extents.y * dist(rnd),
                m_Box.Extents.z * dist(rnd));
    result.direction = Vector3(dir, 0.0f, 0.0f);
    break;
  case 1:
    result.position =
        Vector3(m_Box.Extents.x * dist(rnd), m_Box.Extents.y * dir,
                m_Box.Extents.z * dist(rnd));
    result.direction = Vector3(0.0f, dir, 0.0f);
    break;
  case 2:
    result.position =
        Vector3(m_Box.Extents.x * dist(rnd), m_Box.Extents.y * dist(rnd),
                m_Box.Extents.z * dir);
    result.direction = Vector3(0.0f, 0.0f, dir);
    break;
  default:
    assert(false);
    break;
  }

  auto transform = GetTransform();
  result.position = Vector3::Transform(result.position, transform);
  result.direction = Vector3::TransformNormal(result.direction, transform);
  return result;
}

bool Box::Intersect(const Ray &_ray, Intersection &_intersect) {
  float dist;
  Ray ray = _ray;
	auto transform = GetTransform();
	//ray.position = Vector3::Transform(_ray.position, transform);
  //ray.direction = Vector3::TransformNormal(_ray.direction, transform);
  if (ray.Intersects(m_Box, dist)) {
    if (dist < 0.001f)
      return false;

    _intersect.position = _ray.position + dist * _ray.direction;

    // Calculate normal (based on which axis the intersection point lies most)
    Vector3 fromCenter =
        _intersect.position -
        Vector3(m_Box.Center.x, m_Box.Center.y, m_Box.Center.z);
    fromCenter.x /= m_Box.Extents.x;
    fromCenter.y /= m_Box.Extents.y;
    fromCenter.z /= m_Box.Extents.z;

    float dotX = fromCenter.Dot(Vector3(1, 0, 0));
    float dotY = fromCenter.Dot(Vector3(0, 1, 0));
    float dotZ = fromCenter.Dot(Vector3(0, 0, 1));

    float absX = abs(dotX);
    float absY = abs(dotY);
    float absZ = abs(dotZ);
    float largest = absX;

    if (absY > largest)
      largest = absY;
    if (absZ > largest)
      largest = absZ;

    if (largest == absX)
      _intersect.normal = dotX * Vector3(1, 0, 0);
    else if (largest == absY)
      _intersect.normal = dotY * Vector3(0, 1, 0);
    else if (largest == absZ)
      _intersect.normal = dotZ * Vector3(0, 0, 1);
    _intersect.normal.Normalize();

    _intersect.material = GetMaterial();

    return true;
  }
  return false;
}

