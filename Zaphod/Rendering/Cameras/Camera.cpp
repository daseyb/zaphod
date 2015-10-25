#include "Camera.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;

void Camera::SetPosition(Vector3 _pos) { m_Position = _pos; }

void Camera::SetRotation(Vector3 _rot) {
  m_Rotation = Quaternion::CreateFromRotationMatrix(
      Matrix::CreateRotationX(_rot.x) * Matrix::CreateRotationY(_rot.y) *
      Matrix::CreateRotationZ(_rot.z));
}

void Camera::LookAt(DirectX::SimpleMath::Vector3 _eye,
                    DirectX::SimpleMath::Vector3 _target,
                    DirectX::SimpleMath::Vector3 _up) {
  Matrix lookAtMat = Matrix::CreateLookAt(_eye, _target, _up);
  m_Rotation = Quaternion::CreateFromRotationMatrix(lookAtMat);
  m_Position = lookAtMat.Translation();
}

Matrix Camera::GetViewMatrix(void) const {
  Matrix viewMat = Matrix::CreateFromQuaternion(m_Rotation) *
                   Matrix::CreateTranslation(m_Position);
  viewMat.Invert();
  return viewMat;
}

Camera::~Camera(void) {}
