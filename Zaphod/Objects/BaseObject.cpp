#include "BaseObject.h"

using namespace DirectX::SimpleMath;

BaseObject::BaseObject(void)
{
	m_Scale = Vector3(1, 1, 1);
	m_Rotation = Quaternion::CreateFromYawPitchRoll(0, 0, 0);
	m_Position = Vector3(0, 0, 0);
}

Matrix BaseObject::GetTransform() const
{
	return Matrix::CreateScale(m_Scale) * Matrix::CreateFromQuaternion(m_Rotation) * Matrix::CreateTranslation(m_Position);
}

void BaseObject::SetMaterial(Material _mat)
{
	m_Material = _mat;
}

void BaseObject::SetRotation(Vector3 _rot)
{
  m_Rotation = Quaternion::CreateFromYawPitchRoll(_rot.x, _rot.y, _rot.z);
}

void BaseObject::SetPosition(Vector3 _pos)
{
	m_Position = _pos;
}

void BaseObject::SetScale(Vector3 _scale) {
  m_Scale = _scale;
}

BaseObject::~BaseObject(void)
{
}