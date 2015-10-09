#include "BaseObject.h"
#include "../Rendering/Materials/Material.h"
#include "../Rendering/Materials/EmissionMaterial.h"

using namespace DirectX::SimpleMath;

BaseObject::BaseObject(void)
{
	m_Scale = Vector3(1, 1, 1);
	m_Rotation = Quaternion::CreateFromYawPitchRoll(0, 0, 0);
	m_Position = Vector3(0, 0, 0);
	m_Material = nullptr;
}

Matrix BaseObject::GetTransform() const
{
	return Matrix::CreateScale(m_Scale) * Matrix::CreateFromQuaternion(m_Rotation) * Matrix::CreateTranslation(m_Position);
}

void BaseObject::SetMaterial(Material* _mat)
{
	m_Material.reset(_mat->Copy());
}

float BaseObject::GetWeight() const
{
	return m_Weight;
}

Material* BaseObject::GetMaterial() const
{
	return m_Material.get();
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