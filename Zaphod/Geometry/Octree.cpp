#include "Octree.h"
#include "Triangle.h"
#include "..\Node.h"

using namespace DirectX::SimpleMath;

Octree::Octree(const std::vector<Triangle>& _polys) {
	float minX = FLT_MAX, maxX = FLT_MIN;
	float minY = FLT_MAX, maxY = FLT_MIN;
	float minZ = FLT_MAX, maxZ = FLT_MIN;

	std::vector<const Triangle*> polyPtrs(_polys.size());

	for(size_t i = 0; i < _polys.size(); i++) {
		Triangle tri = _polys[i];
		for(int j = 0; j < 3; j++) {
			Vertex v = tri.v(j);
			if(v.Position.x > maxX)
				maxX = v.Position.x;
			if(v.Position.x < minX)
				minX = v.Position.x;

			if(v.Position.y > maxY)
				maxY = v.Position.y;
			if(v.Position.y < minY)
				minY = v.Position.y;

			if(v.Position.z > maxZ)
				maxZ = v.Position.z;
			if(v.Position.z < minZ)
				minZ = v.Position.z;
		}

		polyPtrs[i] = &_polys[i];
	}

	Vector3 min = Vector3(minX, minY, minZ);
	Vector3 max = Vector3(maxX, maxY, maxZ);

	Vector3 extends = (max - min )/2;
	Vector3 mid = (min + max)/2;


	m_Root = std::make_unique<Node>(DirectX::BoundingBox(mid, extends), polyPtrs, 0);
}

bool Octree::Intersect(const Ray& _ray, Triangle& _out, float& _outDist) const {
	return m_Root->Intersect(_ray, _out, _outDist);
}

Octree::~Octree(void) {

}