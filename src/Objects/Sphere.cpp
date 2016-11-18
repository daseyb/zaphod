#include "Sphere.h"
#include "../Geometry/Intersection.h"

using namespace DirectX::SimpleMath;

Sphere::Sphere(Vector3 _position, float _radius, BaseObject* parent) : RenderObject(parent) {
  m_Sphere = DirectX::BoundingSphere(_position, _radius);
  SetRadius(_radius);
  SetPosition(_position);
}

void Sphere::SetRadius(float _radius) { m_Sphere.Radius = _radius; }

void Sphere::SetPosition(DirectX::SimpleMath::Vector3 _pos) {
  BaseObject::SetPosition(_pos);
}

bool Sphere::Intersect(const Ray &_ray, Intersection &_intersect) {
  float dist;
  auto objToWorld = GetTransform();
  auto worldToObj = objToWorld.Invert();
  Ray ray = _ray;
  ray.position = Vector3::Transform(_ray.position, worldToObj);
  ray.direction = Vector3::TransformNormal(_ray.direction, worldToObj);
  ray.direction.Normalize();
  if (ray.Intersects(m_Sphere, dist)) {
    if (dist < 0.001f)
      return false;

    _intersect.position = ray.position + dist * ray.direction;
    // Calculate normal based on direction from the center to the intersection
    // point
    _intersect.normal =
        _intersect.position -
        Vector3(m_Sphere.Center.x, m_Sphere.Center.y, m_Sphere.Center.z);
    _intersect.normal.Normalize();
    _intersect.material = GetMaterial();


    _intersect.position = Vector3::Transform(_intersect.position, objToWorld);
    _intersect.normal = Vector3::TransformNormal(_intersect.normal, objToWorld);
    _intersect.normal.Normalize();
    return true;
  }
  return false;
}

float Sphere::CalculateWeight() {
  m_Weight = 4 * m_Sphere.Radius * DirectX::XM_PI * DirectX::XM_PI;
  return m_Weight;
}

Ray Sphere::Sample(std::default_random_engine &rnd) {
  std::uniform_real_distribution<float> dist(0, 1);
  float omega = dist(rnd) * DirectX::XM_2PI;
  float phi = dist(rnd) * DirectX::XM_PI;

  Ray result;
  result.direction =
      Vector3(sinf(phi) * cosf(omega), sinf(phi) * sinf(omega), cosf(phi));
  result.position = result.direction * m_Sphere.Radius;

  auto transform = GetTransform();
  result.position = Vector3::Transform(result.position, transform);
  result.direction = Vector3::TransformNormal(result.direction, transform);
  return result;
}