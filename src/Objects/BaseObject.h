#pragma once
#include "../SimpleMath.h"
#include <random>
#include <memory>

struct Intersection;
struct Material;
struct Triangle;

/********************************************
** BaseObject
** Base class for all scene objects. Describes
** a primitive and provides transformation
** functionality.
*********************************************/

class BaseObject {
protected:
  DirectX::SimpleMath::Vector3 m_Scale;
  DirectX::SimpleMath::Vector3 m_Position;
  DirectX::SimpleMath::Quaternion m_Rotation;
  std::unique_ptr<Material> m_Material;
  float m_Weight;

public:
  BaseObject(void);
  ~BaseObject(void);

  DirectX::SimpleMath::Matrix GetTransform() const;
  virtual void SetRotation(DirectX::SimpleMath::Vector3 _rot);
  virtual void SetPosition(DirectX::SimpleMath::Vector3 _pos);
  virtual void SetScale(DirectX::SimpleMath::Vector3 _scale);
  virtual float CalculateWeight() = 0;
  virtual DirectX::SimpleMath::Ray
  Sample(std::default_random_engine &rnd) const = 0;

  float GetWeight() const;

  void SetMaterial(Material *_mat);

  Material *GetMaterial() const;

  virtual bool Intersect(const DirectX::SimpleMath::Ray &_ray,
                         Intersection &_intersect) const = 0;

  virtual bool HasBuffers() const { return false; }
  virtual const DirectX::SimpleMath::Vector3* GetVertexBuffer() const { return nullptr; }
  virtual const Triangle* GetIndexBuffer() const { return nullptr; }
  virtual size_t GetVertexCount() const { return 0; }
  virtual size_t GetTriangleCount() const { return 0; }
};
