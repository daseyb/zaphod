#pragma once
#include "RenderObject.h"
#include <vector>
#include <memory>
#include "../Geometry/Triangle.h"

class Mesh : public RenderObject {
private:
  std::vector<Triangle> m_Triangles;
  std::vector<DirectX::SimpleMath::Vector3> m_Vertices;
  std::vector<DirectX::SimpleMath::Vector3> m_Normals;
  std::vector<DirectX::SimpleMath::Vector2> m_UVs;

  bool m_Smooth;

public:
  Mesh::Mesh(DirectX::SimpleMath::Vector3 _pos, std::vector<Triangle> &_tris,
             std::vector<DirectX::SimpleMath::Vector3> &_verts, std::vector<DirectX::SimpleMath::Vector3> &_normals,
             std::vector<DirectX::SimpleMath::Vector2> &_uvs, bool _smooth, BaseObject* parent = nullptr);
  ~Mesh(void);
  bool Intersect(const DirectX::SimpleMath::Ray &_ray,
                 Intersection &_intersect) override;

  float CalculateWeight() override;

  DirectX::SimpleMath::Ray
  Sample(std::default_random_engine &rnd) override;

  virtual bool HasBuffers() const { return true; }
  virtual const DirectX::SimpleMath::Vector3 *GetVertexBuffer() const override {
    return m_Vertices.data();
  }
  virtual const Triangle *GetIndexBuffer() const override {
    return m_Triangles.data();
  }
  virtual size_t GetVertexCount() const override { return m_Vertices.size(); }
  virtual size_t GetTriangleCount() const override { return m_Triangles.size(); }
};
