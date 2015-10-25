#include "Node.h"
#include "Triangle.h"
#include <algorithm>

using namespace DirectX::SimpleMath;

Node::Node(DirectX::BoundingBox _bounds, std::vector<const Triangle *> _polys,
           int _depth) {
  m_Bounds = _bounds;
  m_Smallest = false;
  m_Depth = _depth;

  m_Polys = _polys;
  m_Children = std::vector<Node *>();

  if (m_Bounds.Extents.x * m_Bounds.Extents.y * m_Bounds.Extents.z * 8 >
          MIN_NODE_SIZE &&
      m_Depth < MAX_DEPTH && _polys.size() > MIN_LEAF_COUNT) {
    Divide();
  } else {
    m_Smallest = true;
  }
}

Node::~Node(void) {
  for (auto child = m_Children.begin(), end = m_Children.end(); child != end;
       ++child) {
    delete (*child);
  }
}

Vector3 Node::GetCenter() const {
  return Vector3(m_Bounds.Center.x, m_Bounds.Center.y, m_Bounds.Center.z);
}

bool Node::Contains(const DirectX::BoundingBox &_bounds,
                    const Triangle &_poly) const {
  return _bounds.Contains(_poly.v(0).Position, _poly.v(1).Position,
                          _poly.v(2).Position);
}

bool Node::Intersect(const Ray &_ray, Triangle &_outTri,
                     float &_outDist) const {
  float dist = FLT_MAX;
  bool intersectFound = false;
  _outDist = FLT_MAX;
  if (_ray.Intersects(m_Bounds, dist)) {
    dist = FLT_MAX;
    Triangle tri;

    if (m_Smallest) {
      // Test all triangles for intersection
      for (size_t i = 0; i < m_Polys.size(); i++) {
        tri = *m_Polys[i];
        if (_ray.Intersects(tri.v(0).Position, tri.v(1).Position,
                            tri.v(2).Position, dist) &&
            dist > 0.001f && dist < _outDist) {
          _outTri = tri;
          _outDist = dist;
          intersectFound = true;
        }
      }
    } else {
      // Test all children
      for (size_t i = 0; i < m_Children.size(); i++) {
        if (m_Children[i]->Intersect(_ray, tri, dist) && dist < _outDist) {
          _outTri = tri;
          _outDist = dist;
          intersectFound = true;
        }
      }
    }
  }
  return intersectFound;
}

Vector3 Node::GetChildPos(int _index) {
  float sizeX = m_Bounds.Extents.x / 2;
  float sizeY = m_Bounds.Extents.y / 2;
  float sizeZ = m_Bounds.Extents.z / 2;

  Vector3 pos =
      Vector3(m_Bounds.Center.x, m_Bounds.Center.y, m_Bounds.Center.z);

  switch (_index) {
  case 0:
    return Vector3(sizeX, sizeY, sizeZ) + pos; // top	top		top
  case 1:
    return Vector3(sizeX, sizeY, -sizeZ) + pos; // top	top		bottom
  case 2:
    return Vector3(sizeX, -sizeY, sizeZ) + pos; // top	bottom	top
  case 3:
    return Vector3(sizeX, -sizeY, -sizeZ) + pos; // top	bottom	bottom
  case 4:
    return Vector3(-sizeX, sizeY, sizeZ) + pos; // bottom	top
                                                // top
  case 5:
    return Vector3(-sizeX, sizeY, -sizeZ) +
           pos; // bottom	top		bottom
  case 6:
    return Vector3(-sizeX, -sizeY, sizeZ) + pos; // bottom	bottom	top
  case 7:
    return Vector3(-sizeX, -sizeY, -sizeZ) + pos; // bottom	bottom	bottom
  default:
    assert(false);
  }
}

void Node::Divide() {
  Vector3 extends;
  extends.x = m_Bounds.Extents.x / 2;
  extends.y = m_Bounds.Extents.y / 2;
  extends.z = m_Bounds.Extents.z / 2;

  for (int i = 0; i < 8; i++) {
    DirectX::BoundingBox childBounds(GetChildPos(i), extends);

    auto childPolys = std::vector<const Triangle *>();

    for (int j = 0; j < m_Polys.size(); j++) {
      auto poly = *m_Polys[j];
      if (childBounds.Contains(poly.v(0).Position, poly.v(1).Position,
                               poly.v(2).Position)) {
        childPolys.push_back(m_Polys[j]);
      }
    }

    if (childPolys.size() > 0) {
      m_Children.push_back(new Node(childBounds, childPolys, m_Depth + 1));
    }
  }

  m_Polys.clear();
}
