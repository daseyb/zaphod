#pragma once
#include "..\SimpleMath.h"

struct Vertex
{
	DirectX::SimpleMath::Vector3 Position;
	DirectX::SimpleMath::Vector3 Normal;
	DirectX::SimpleMath::Vector2 UV;
};

class Triangle
{
private:
	Vertex m_Vertices[3];
public:
	Triangle();
	Triangle(Vertex _v1, Vertex _v2, Vertex _v3);
	~Triangle(void);
	Vertex v(int _index) const;
};

