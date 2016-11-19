#pragma once
#include "../SimpleMath.h"
#include <random>
#include <memory>
#include "Timeline.h"

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
  BaseObject *m_Parent;
  bool m_TransformIsDirty;
  DirectX::SimpleMath::Matrix m_Transform;

  Timeline<DirectX::SimpleMath::Vector3> m_PositionTimeline;
  Timeline<DirectX::SimpleMath::Quaternion> m_RotationTimeline;
  Timeline<DirectX::SimpleMath::Vector3> m_ScaleTimeline;

protected:
  DirectX::SimpleMath::Vector3 m_Scale;
  DirectX::SimpleMath::Vector3 m_Position;
  DirectX::SimpleMath::Quaternion m_Rotation;
  float m_Weight = 0;
  int currentFrame = 0;

public:
  BaseObject(BaseObject *_parent);

  bool IsTransformDirty() const {
    return m_TransformIsDirty || (m_Parent && m_Parent->IsTransformDirty());
  }
  DirectX::SimpleMath::Matrix GetTransform();
  void SetParent(BaseObject *parent);
  float GetWeight() const;

  void SetPositionTimeline(Timeline<DirectX::SimpleMath::Vector3> posTimeline) {
    m_PositionTimeline = posTimeline;
    SetPosition(m_PositionTimeline.EvaluateAtFrame(currentFrame));
  }

  void
  SetRotationTimeline(Timeline<DirectX::SimpleMath::Quaternion> rotTimeline) {
    m_RotationTimeline = rotTimeline;
    SetRotation(m_RotationTimeline.EvaluateAtFrame(currentFrame));
  }

  void SetScaleTimeline(Timeline<DirectX::SimpleMath::Vector3> scaleTimeline) {
    m_ScaleTimeline = scaleTimeline;
    SetScale(m_ScaleTimeline.EvaluateAtFrame(currentFrame));
  }

  virtual void SetRotation(DirectX::SimpleMath::Vector3 _rot);
  virtual void SetRotation(DirectX::SimpleMath::Quaternion _rot);
  virtual void SetPosition(DirectX::SimpleMath::Vector3 _pos);
  virtual void SetScale(DirectX::SimpleMath::Vector3 _scale);

  virtual void SetTime(int frameIndex) {
    currentFrame = frameIndex;
    if (m_PositionTimeline.keyframes.size())
      SetPosition(m_PositionTimeline.EvaluateAtFrame(frameIndex));
    if (m_RotationTimeline.keyframes.size())
      SetRotation(m_RotationTimeline.EvaluateAtFrame(frameIndex));
    if (m_ScaleTimeline.keyframes.size())
      SetScale(m_ScaleTimeline.EvaluateAtFrame(frameIndex));
  }
};
