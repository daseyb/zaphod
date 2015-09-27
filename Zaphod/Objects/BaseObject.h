#pragma once

#include "../SimpleMath.h"
#include "../Material.h"

struct Intersection;

/********************************************
** BaseObject
** Base class for all scene objects. Describes
** a primitive and provides transformation
** functionality.
*********************************************/

class BaseObject
{
protected:
	DirectX::SimpleMath::Vector3 m_Scale;
	DirectX::SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Quaternion m_Rotation;
	Material m_Material;

public: 
	BaseObject(void);
	~BaseObject(void);
	
	DirectX::SimpleMath::Matrix GetTransform() const;
  virtual void SetRotation(DirectX::SimpleMath::Vector3 _rot);
  virtual void SetPosition(DirectX::SimpleMath::Vector3 _pos);
  virtual void SetScale(DirectX::SimpleMath::Vector3 _scale);
  void SetMaterial(Material _mat);
	virtual bool Intersect(const DirectX::SimpleMath::Ray& _ray, Intersection& _intersect) const = 0;
};

