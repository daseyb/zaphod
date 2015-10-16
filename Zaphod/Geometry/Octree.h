#pragma once
#include <vector>
#include "..\SimpleMath.h"
#include <memory>

class Triangle;
class Node;

class Octree
{
private:
	std::unique_ptr<Node> m_Root;

public:
	Octree(const std::vector<Triangle>& _polys);
	~Octree(void);
	bool Intersect(const DirectX::SimpleMath::Ray& _ray, Triangle& _out, float& _outDist) const;
};

