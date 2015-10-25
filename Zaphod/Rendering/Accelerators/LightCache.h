#pragma once
#include "../../SimpleMath.h"

class LightCacheNode;

class LightCache {
private:
  LightCacheNode *root;

public:
  LightCache(DirectX::BoundingBox _bounds);
  ~LightCache();
  void AddPoint(DirectX::SimpleMath::Vector3 _point,
                DirectX::SimpleMath::Color _color);
  bool LookUp(DirectX::SimpleMath::Vector3 _point,
              DirectX::SimpleMath::Color *_outCol, int *_storedCount);
};
