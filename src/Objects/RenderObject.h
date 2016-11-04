#pragma once
#include "../SimpleMath.h"
#include <random>
#include <memory>
#include "BaseObject.h"
#include "../Rendering/Materials/Material.h"

struct Intersection;
struct Material;
struct Triangle;

class RenderObject : public BaseObject {
private:
	std::unique_ptr<Material> m_Material;

public:
	RenderObject(BaseObject* _parent);

  virtual float CalculateWeight() = 0;
  virtual DirectX::SimpleMath::Ray Sample(std::default_random_engine &rnd) = 0;
  virtual bool Intersect(const DirectX::SimpleMath::Ray &_ray,
                         Intersection &_intersect) = 0;

  virtual bool HasBuffers() const { return false; }
  virtual const DirectX::SimpleMath::Vector3* GetVertexBuffer() const { return nullptr; }
  virtual const Triangle* GetIndexBuffer() const { return nullptr; }
  virtual size_t GetVertexCount() const { return 0; }
  virtual size_t GetTriangleCount() const { return 0; }

	void SetMaterial(Material *_mat);
	Material *GetMaterial() const;
};
