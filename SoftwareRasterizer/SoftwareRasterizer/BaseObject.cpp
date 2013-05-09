#include "BaseObject.h"

using namespace DirectX::SimpleMath;

BaseObject::BaseObject(void)
{
	m_Scale = Vector3(1, 1, 1);
	m_Rotation = Quaternion::CreateFromYawPitchRoll(0, 0, 0);
	m_Position = Vector3(0, 0, 0);
	m_Diffuse = Color(1,1,1);
}

Matrix BaseObject::GetTransform()
{
	return Matrix::CreateScale(m_Scale) * Matrix::CreateFromQuaternion(m_Rotation) * Matrix::CreateTranslation(m_Position);
}

void BaseObject::SetDiffuse(DirectX::SimpleMath::Color _col)
{
	m_Diffuse = _col;
}

void BaseObject::SetPosition(Vector3 _pos)
{
	m_Position = _pos;
}

BaseObject::~BaseObject(void)
{
}