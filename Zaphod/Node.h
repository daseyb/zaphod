#pragma once
#include <vector>
#include "SimpleMath.h"

class Triangle;
const float MIN_NODE_SIZE = 0.01f;
const int MAX_DEPTH = 12;
const int MIN_LEAF_COUNT = 8;

class Node {
private:
	DirectX::BoundingBox m_Bounds;
	bool m_Smallest;
  int m_Depth;

	std::vector<Node*> m_Children;
	std::vector<const Triangle*> m_Polys;

	DirectX::SimpleMath::Vector3 GetChildPos(int _index);
	void Divide();
public:
	Node(DirectX::BoundingBox _bounds, std::vector<const Triangle*> _parentPolys, int _depth);
	~Node(void);
	DirectX::SimpleMath::Vector3 GetCenter() const;
	bool Contains(const DirectX::BoundingBox& _bounds, const Triangle& _poly) const;
	bool Intersect(const DirectX::SimpleMath::Ray& _ray, Triangle& _out, float& _dist) const;
};
