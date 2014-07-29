#pragma once
#include "BaseObject.h"
#include <vector>
#include "../Geometry/Octree.h"
#include <memory>

class Triangle;
class Mesh :
	public BaseObject
{
private:
	std::vector<Triangle> m_Triangles;
	std::unique_ptr<Octree> m_Bounds;
	bool m_Smooth;
public:
	Mesh(DirectX::SimpleMath::Vector3 _pos, const std::string& _file);
	Mesh(DirectX::SimpleMath::Vector3 _pos, std::vector<Triangle> _tris, bool _smooth);
	~Mesh(void);
	void SetPosition(DirectX::SimpleMath::Vector3 _pos) override;
	void SetRotation(float _yaw, float _pitch, float _roll);
	bool Intersect(const DirectX::SimpleMath::Ray& _ray, Intersection& _intersect) const override;
};

