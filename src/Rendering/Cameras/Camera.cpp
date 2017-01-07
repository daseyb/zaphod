#include "Camera.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;

void Camera::SetPosition(Vector3 _pos) { 
  m_Position = _pos; 
  m_ViewMatrix = (Matrix::CreateFromQuaternion(m_Rotation) *
    Matrix::CreateTranslation(m_Position));
}

void Camera::SetRotation(Vector3 _rot) {
  m_Rotation = Quaternion::CreateFromRotationMatrix(
      Matrix::CreateRotationX(_rot.x) * Matrix::CreateRotationY(_rot.y) *
      Matrix::CreateRotationZ(_rot.z));

  m_ViewMatrix = (Matrix::CreateFromQuaternion(m_Rotation) *
    Matrix::CreateTranslation(m_Position));
}

void Camera::LookAt(DirectX::SimpleMath::Vector3 _eye,
                    DirectX::SimpleMath::Vector3 _target,
                    DirectX::SimpleMath::Vector3 _up) {
  Matrix lookAtMat = Matrix::CreateLookAt(_eye, _target, _up);
  m_Rotation = Quaternion::CreateFromRotationMatrix(lookAtMat);
  m_Position = lookAtMat.Translation();

  m_ViewMatrix = (Matrix::CreateFromQuaternion(m_Rotation) *
    Matrix::CreateTranslation(m_Position));
}

Matrix Camera::GetViewMatrix(void) const {
  return m_ViewMatrix;
}

void Camera::SetViewMatrix(DirectX::SimpleMath::Matrix matrix) {
    Vector3 scale;
    matrix.Decompose(scale, m_Rotation, m_Position);

    m_ViewMatrix = (Matrix::CreateFromQuaternion(m_Rotation) *
      Matrix::CreateTranslation(m_Position));
}

Camera::~Camera(void) {}
