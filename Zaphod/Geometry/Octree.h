#pragma once
#include <vector>
#include "..\SimpleMath.h"

class Triangle;
class Node;

class Octree
{
private:
	Node* m_Root;

public:
	Octree(void);
	Octree(std::vector<Triangle> _polys, DirectX::SimpleMath::Vector3 _center);
	~Octree(void);
	bool Intersect(const DirectX::SimpleMath::Ray& _ray, Triangle& _out, float& _outDist) const;
};

