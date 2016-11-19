#pragma once

#include "../SimpleMath.h"
#include <algorithm>
#include <vector>

template <typename ValueType> struct Keyframe {
  int FrameIndex;
  ValueType Value;
};

template <typename ValueType>
ValueType interpolate(const ValueType &a, const ValueType &b, float t) {
  return a + (b - a) * t;
}

template <>
inline DirectX::SimpleMath::Quaternion interpolate<DirectX::SimpleMath::Quaternion>(
    const DirectX::SimpleMath::Quaternion &a,
    const DirectX::SimpleMath::Quaternion &b, float t) {
  return DirectX::SimpleMath::Quaternion::Slerp(a, b, t);
}

template <typename ValueType> struct Timeline {
  std::vector<Keyframe<ValueType>> keyframes;

  ValueType EvaluateAtFrame(int index) {
    auto it = std::find_if(std::begin(keyframes), std::end(keyframes),
                           [index](Keyframe<ValueType> frame) {
                             return frame.FrameIndex > index;
                           });
    if (it == keyframes.end())
      return keyframes[keyframes.size() - 1].Value;
    if (it - std::begin(keyframes) == 0)
      return keyframes[0].Value;

    auto startFrame = it - 1;
    auto endFrame = it;

    float t = float(index - startFrame->FrameIndex) /
              (endFrame->FrameIndex - startFrame->FrameIndex);

    return interpolate(startFrame->Value, endFrame->Value, t);
  }
};