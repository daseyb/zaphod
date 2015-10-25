#pragma once
#include "..\SimpleMath.h"

struct Vertex {
  DirectX::SimpleMath::Vector3 Position;
  DirectX::SimpleMath::Vector3 Normal;
  DirectX::SimpleMath::Vector2 UV;

  bool operator==(const Vertex &rhs) const {
    return Position == rhs.Position && Normal == rhs.Normal && UV == rhs.UV;
  }
};

class Triangle {
private:
  Vertex m_Vertices[3];

public:
  Triangle();
  Triangle(Vertex _v1, Vertex _v2, Vertex _v3);
  bool operator==(const Triangle &rhs) const;
  ~Triangle(void);
  inline Vertex v(int _index) const { return m_Vertices[_index]; }
};
