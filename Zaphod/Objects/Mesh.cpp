#include "Mesh.h"
#include "..\Geometry\Intersection.h"
#include "..\Geometry\Triangle.h"

using namespace DirectX::SimpleMath;

Mesh::Mesh(Vector3 _pos, std::vector<Triangle>& _tris, std::vector<Vector3>& _verts, std::vector<Vector3>& _normals, std::vector<Vector2>& _uvs, bool _smooth) {
  m_Triangles = std::move(_tris);
  m_Vertices = std::move(_verts);
  m_Normals = std::move(_normals);
  m_UVs = std::move(_uvs);

  m_Smooth = _smooth;
  SetPosition(_pos);
}

Mesh::~Mesh(void) {}

bool Mesh::Intersect(const Ray &_ray, Intersection &_intersect) const {
  return false;
}

float Mesh::CalculateWeight() {
  m_Weight = 0;
  return 0;
}

Ray Mesh::Sample(std::default_random_engine &rnd) const { return Ray(); }
