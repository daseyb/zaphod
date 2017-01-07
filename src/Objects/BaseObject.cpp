#include "BaseObject.h"
#include "../Rendering/Materials/EmissionMaterial.h"

using namespace DirectX::SimpleMath;

BaseObject::BaseObject(BaseObject *parent) : m_Parent(parent) {
  m_Scale = Vector3(1, 1, 1);
  m_Rotation = Quaternion::CreateFromYawPitchRoll(0, 0, 0);
  m_Position = Vector3(0, 0, 0);
  m_TransformIsDirty = true;
  m_Transform = GetTransform();

}

Matrix BaseObject::GetTransformInv() {
	if (!IsTransformDirty()) {
		return m_TransformInverse;
	}

	m_TransformInverse = GetTransform().Invert();

	return m_TransformInverse;
}

Matrix BaseObject::GetTransform() {
  if (!IsTransformDirty()) {
    return m_Transform;
  }

  Matrix parentTransform =
      m_Parent ? m_Parent->GetTransform() : Matrix::Identity();
  m_Transform = Matrix::CreateScale(m_Scale) *
                Matrix::CreateFromQuaternion(m_Rotation) *
                Matrix::CreateTranslation(m_Position) * parentTransform;

  m_TransformIsDirty = false;
	m_TransformInverse = m_Transform.Invert();

  return m_Transform;
}

float BaseObject::GetWeight() const { return m_Weight; }

void BaseObject::SetRotation(Vector3 _rot) {
  m_Rotation = Quaternion::CreateFromAxisAngle(Vector3(1, 0, 0), _rot.x) *
               Quaternion::CreateFromAxisAngle(Vector3(0, 1, 0), _rot.y) *
               Quaternion::CreateFromAxisAngle(Vector3(0, 0, 1), _rot.z);
  m_TransformIsDirty = true;
}

void BaseObject::SetRotation(Quaternion _rot) {
  m_Rotation = _rot;
  m_TransformIsDirty = true;
}

void BaseObject::SetPosition(Vector3 _pos) {
  m_Position = _pos;
  m_TransformIsDirty = true;
}

void BaseObject::SetScale(Vector3 _scale) {
  m_Scale = _scale;
  m_TransformIsDirty = true;
}
void BaseObject::SetParent(BaseObject *parent) {
  m_Parent = parent;
  m_TransformIsDirty = true;
}