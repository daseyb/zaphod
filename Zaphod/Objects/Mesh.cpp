#include "Mesh.h"
#include "..\Geometry\Intersection.h"
#include "..\Geometry\Triangle.h"
#include "../IO/ObjLoader.h"

using namespace DirectX::SimpleMath;

Mesh::Mesh(DirectX::SimpleMath::Vector3 _pos, const std::string &_file) {
  if (LoadObj(_file, m_Triangles, m_Smooth)) {
    assert(m_Triangles.size() > 0);
    m_Bounds = std::unique_ptr<Octree>(new Octree(m_Triangles));
    SetPosition(_pos);
  }
}

Mesh::Mesh(Vector3 _pos, std::vector<Triangle> _tris, bool _smooth) {
  m_Bounds = std::unique_ptr<Octree>(new Octree(m_Triangles));
  m_Smooth = _smooth;
  SetPosition(_pos);
}

Mesh::~Mesh(void) {}

bool Mesh::Intersect(const Ray &_ray, Intersection &_intersect) const {
  float minDist = FLT_MAX;
  Triangle minTri;
  Ray transformedRay = _ray;

  auto transform = GetTransform();
  auto invTransform = transform.Invert();

  transformedRay.position = Vector3::Transform(_ray.position, invTransform);
  transformedRay.direction =
      Vector3::TransformNormal(_ray.direction, invTransform);
  transformedRay.direction.Normalize();

  bool intersectFound = m_Bounds->Intersect(transformedRay, minTri, minDist);

  if (intersectFound) {
    _intersect.material = m_Material.get();
    _intersect.position = _ray.position + minDist * _ray.direction;
    _intersect.normal = Vector3::TransformNormal(
        (minTri.v(0).Normal + minTri.v(1).Normal + minTri.v(2).Normal) / 3,
        transform);
    _intersect.normal.Normalize();
  }
  return intersectFound;
}

float Mesh::CalculateWeight() {
  m_Weight = 0;
  return 0;
}

Ray Mesh::Sample(std::default_random_engine &rnd) const { return Ray(); }
