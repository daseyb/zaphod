#pragma once
#include <vector>
#include "SimpleMath.h"

class Triangle;
const int MIN_POLYS_PER_NODE = 10;

class Node {
private:
	DirectX::BoundingBox m_Bounds;

	bool m_Smallest;
	std::vector<Node*> m_Children;
	std::vector<Triangle> m_Polys;

	DirectX::SimpleMath::Vector3 GetChildPos(int _index);
	void Devide();
public:
	Node(DirectX::BoundingBox _bounds, std::vector<Triangle> _parentPolys);
	~Node(void);
	DirectX::SimpleMath::Vector3 GetCenter() const;
	bool Contains(const Triangle& _poly) const;
	bool Intersect(const DirectX::SimpleMath::Ray& _ray, Triangle& _out, float& _dist) const;
	bool HasPolys(void) const;
};
