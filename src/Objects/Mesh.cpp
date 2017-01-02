#include "Mesh.h"
#include "..\Geometry\Intersection.h"
#include "..\Geometry\Triangle.h"

using namespace DirectX::SimpleMath;

Mesh::Mesh(Vector3 _pos, std::vector<Triangle>& _tris, std::vector<Vector3>& _verts, std::vector<Vector3>& _normals, std::vector<Vector2>& _uvs, bool _smooth, BaseObject* parent) : RenderObject(parent) {
  m_Triangles = std::move(_tris);
  m_Vertices = std::move(_verts);
  m_Normals = std::move(_normals);
  m_UVs = std::move(_uvs);

  m_Smooth = _smooth;
  SetPosition(_pos);
}

Mesh::~Mesh(void) {}

bool Mesh::Intersect(const Ray &_ray, Intersection &_intersect) {
  return false;
}

float TriArea(Vector3 a, Vector3 b, Vector3 c) {
	return (b - a).Cross(c - a).Length() / 2;
}

float Mesh::CalculateWeight() {
	m_Weight = 0;

	std::vector<float> triWeights;
	triWeights.reserve(m_Triangles.size());

	for (auto tri : m_Triangles) {
		auto a = m_Vertices[tri.m_Indices[0]];
		auto b = m_Vertices[tri.m_Indices[1]];
		auto c = m_Vertices[tri.m_Indices[2]];

		float weight = TriArea(a, b, c);
		triWeights.push_back(weight);

		m_Weight += weight;
	}

	m_TriSampleWeights = std::discrete_distribution<>(triWeights.begin(), triWeights.end());

  return m_Weight;
}

Ray Mesh::Sample(std::default_random_engine &rnd) { 
	auto triIndex = m_TriSampleWeights(rnd);

	auto tri = m_Triangles[triIndex];

	std::uniform_real_distribution<float> dist(0, 1);

	float u = dist(rnd);
	float v = dist(rnd);

	if (u + v > 1) {
		u = 1.0f - u;
		v = 1.0f - v;
	}

	auto a = m_Vertices[tri.m_Indices[0]];
	auto b = m_Vertices[tri.m_Indices[1]];
	auto c = m_Vertices[tri.m_Indices[2]];

	auto ab = b - a;
	auto ac = c - a;

	auto pos = a + ab * u + ac * v;

	auto norm = ab.Cross(ac);
	norm.Normalize();

	auto transform = GetTransform();

	Ray r;

	r.position = Vector3::Transform(pos, transform);
	r.direction = Vector3::TransformNormal(norm, transform);

	return r;
}
