#include "Mesh.h"
#include "..\Geometry\Intersection.h"
#include "..\Geometry\Triangle.h"
#include "..\ObjLoader.h"

using namespace DirectX::SimpleMath;

Mesh::Mesh(DirectX::SimpleMath::Vector3 _pos, const std::string& _file) {
	if(LoadObj(_file, m_Triangles, m_Smooth)) {
		m_Bounds = std::unique_ptr<Octree>(new Octree(m_Triangles));
		SetPosition(_pos);	
	}
}

Mesh::Mesh(Vector3 _pos, std::vector<Triangle> _tris, bool _smooth) {
	m_Bounds = std::unique_ptr<Octree>(new Octree(m_Triangles));
	m_Smooth = _smooth;
	SetPosition(_pos);
}


Mesh::~Mesh(void) {
}

void Mesh::SetPosition(DirectX::SimpleMath::Vector3 _pos) {
	BaseObject::SetPosition(_pos);
}

void Mesh::SetRotation(float _yaw, float _pitch, float _roll) {
	m_Rotation = Quaternion::CreateFromYawPitchRoll(_yaw, _pitch, _roll);
}

bool Mesh::Intersect(const Ray& _ray, Intersection& _intersect) const {
	float minDist = FLT_MAX;
	Triangle minTri;
	Ray transformedRay = _ray;
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationQuaternion(-m_Rotation);
	DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(-m_Position.x, -m_Position.y, -m_Position.z);
	DirectX::XMMATRIX transform = DirectX::XMMatrixMultiply(rotationMatrix, translationMatrix);

	transformedRay.position = Vector3::Transform(_ray.position, transform);

	bool intersectFound = m_Bounds->Intersect(transformedRay, minTri, minDist);

	if(intersectFound) {
		_intersect.material = m_Material;
		_intersect.position = _ray.position + minDist * _ray.direction;
		_intersect.normal = (minTri.v(0).Normal + minTri.v(1).Normal + minTri.v(2).Normal)/3;
	}	
	return intersectFound;
}
