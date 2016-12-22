#pragma once
#include "../../SimpleMath.h"
#include <random>

/********************************************
** Camera
** Implements a simple camera with position
** and rotation. Provides functionality for
** orientating teh viewport.
*********************************************/

class Camera {
private:
  DirectX::SimpleMath::Vector3 m_Position;
  DirectX::SimpleMath::Quaternion m_Rotation;

public:
  Camera() : m_Position(), m_Rotation(){};
  void SetPosition(DirectX::SimpleMath::Vector3 _pos);
  void SetRotation(DirectX::SimpleMath::Vector3 _rot);
  void LookAt(DirectX::SimpleMath::Vector3 _eye,
              DirectX::SimpleMath::Vector3 _target,
              DirectX::SimpleMath::Vector3 _up);
  DirectX::SimpleMath::Matrix GetViewMatrix(void) const;
  void SetViewMatrix(DirectX::SimpleMath::Matrix matrix);

  virtual DirectX::SimpleMath::Ray GetRay(int _x, int _y, int _w, int _h,
                                          std::default_random_engine &_rnd,
                                          float &weight) const = 0;

  ~Camera(void);
};
