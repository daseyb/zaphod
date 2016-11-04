#pragma once
#include "../SimpleMath.h"
#include <random>
#include <memory>

struct Intersection;
struct Triangle;

/********************************************
** BaseObject
** Base class for all scene objects. Describes
** a primitive and provides transformation
** functionality.
*********************************************/

class BaseObject {
private: 
  BaseObject* m_Parent;
	bool m_TransformIsDirty;
	DirectX::SimpleMath::Matrix m_Transform;

protected:
  DirectX::SimpleMath::Vector3 m_Scale;
  DirectX::SimpleMath::Vector3 m_Position;
  DirectX::SimpleMath::Quaternion m_Rotation;
  float m_Weight;

public:
  BaseObject(BaseObject* _parent);

	bool IsTransformDirty() const { return m_TransformIsDirty || (m_Parent && m_Parent->IsTransformDirty()); }
  DirectX::SimpleMath::Matrix GetTransform();
	void SetParent(BaseObject * parent);
	float GetWeight() const;

	virtual void SetRotation(DirectX::SimpleMath::Vector3 _rot);
  virtual void SetPosition(DirectX::SimpleMath::Vector3 _pos);
  virtual void SetScale(DirectX::SimpleMath::Vector3 _scale);

  virtual void SetTime(float time) { }
};
