#include "LightCacheNode.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

LightCacheNode::LightCacheNode(BoundingBox _bounds) {
  for (int i = 0; i < 8; i++) {
    Vector3 extends;
    extends.x = _bounds.Extents.x / 2;
    extends.y = _bounds.Extents.y / 2;
    extends.z = _bounds.Extents.z / 2;

    childBounds[i] = DirectX::BoundingBox(GetChildPos(i, _bounds), extends);
  }

  volume = _bounds.Extents.x * _bounds.Extents.y * _bounds.Extents.z;
  for (int i = 0; i < 8; i++) {
    childs[i] = nullptr;
  }

  storedColors = 0;
  color = Color(0, 0, 0);
}

void LightCacheNode::AddPoint(Vector3 _point, Color _color, float _minSize) {

  storedColors++;
  float mix = 1.0f / storedColors;
  color = color * (1.0f - mix) + _color * mix;

  if (volume < _minSize) {
    return;
  }

  int childIndex = GetChildIndex(_point);

  if (childIndex == -1) {
    return;
  }

  if (!childs[childIndex]) {
    childs[childIndex] = new LightCacheNode(childBounds[childIndex]);
  }

  childs[childIndex]->AddPoint(_point, _color, _minSize);
}

bool LightCacheNode::Lookup(Vector3 _point, float _minSize, Color *_outCol,
                            int *_storedCount) {
  if (volume < _minSize) {
    *_outCol = color;
    *_storedCount = storedColors;
    return true;
  }

  int childIndex = GetChildIndex(_point);

  if (childIndex == -1) {
    return false;
  }

  if (!childs[childIndex]) {
    *_outCol = color;
    *_storedCount = storedColors;
    return true;
  }

  return childs[childIndex]->Lookup(_point, _minSize, _outCol, _storedCount);
}

int LightCacheNode::GetChildIndex(Vector3 _pos) {
  for (int i = 0; i < 8; i++) {
    if (childBounds[i].Contains(_pos)) {
      return i;
    }
  }
  return -1;
}

Vector3 LightCacheNode::GetChildPos(int _index, BoundingBox _bounds) {
  float sizeX = _bounds.Extents.x;
  float sizeY = _bounds.Extents.y;
  float sizeZ = _bounds.Extents.z;

  Vector3 pos = Vector3(_bounds.Center.x, _bounds.Center.y, _bounds.Center.z);

  switch (_index) {
  case 0:
    return (Vector3)(Vector3(sizeX, sizeY, sizeZ) / 2) +
           pos; // top	top		top
  case 1:
    return (Vector3)(Vector3(sizeX, sizeY, -sizeZ) / 2) +
           pos; // top	top		bottom
  case 2:
    return (Vector3)(Vector3(sizeX, -sizeY, sizeZ) / 2) +
           pos; // top	bottom	top
  case 3:
    return (Vector3)(Vector3(sizeX, -sizeY, -sizeZ) / 2) +
           pos; // top	bottom	bottom
  case 4:
    return (Vector3)(Vector3(-sizeX, sizeY, sizeZ) / 2) +
           pos; // bottom	top		top
  case 5:
    return (Vector3)(Vector3(-sizeX, sizeY, -sizeZ) / 2) +
           pos; // bottom	top		bottom
  case 6:
    return (Vector3)(Vector3(-sizeX, -sizeY, sizeZ) / 2) +
           pos; // bottom	bottom	top
  case 7:
    return (Vector3)(Vector3(-sizeX, -sizeY, -sizeZ) / 2) +
           pos; // bottom	bottom	bottom
  }

  return pos;
}