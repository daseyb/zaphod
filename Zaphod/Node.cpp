#include "Node.h"
#include "Geometry\Triangle.h"

using namespace DirectX::SimpleMath;

Node::Node(DirectX::BoundingBox _bounds, std::vector<Triangle> _polys) {
	m_Bounds = _bounds;
	m_Smallest = false;

	m_Polys = std::vector<Triangle>();
	m_Children = std::vector<Node*>();

	for(int i = 0; i < _polys.size(); i++) {
		if(Contains(_polys[i])) {
			m_Polys.push_back(_polys[i]);
		}
	}
	
	if(m_Polys.size() > MIN_POLYS_PER_NODE && false) {
		Devide();
	} else  {
		m_Smallest = true;		
	}
}

Node::~Node(void) {
	for(auto child = m_Children.begin(), end = m_Children.end(); child != end; child++) {
		if(*child) {
			delete (*child);
		}
	}
}

Vector3 Node::GetCenter() const {
	return Vector3(m_Bounds.Center.x, m_Bounds.Center.y, m_Bounds.Center.z);
}

bool Node::Contains(const Triangle& _poly) const {
	return m_Bounds.Intersects(_poly.v(0).Position, _poly.v(1).Position, _poly.v(2).Position);
}

bool Node::Intersect(const Ray& _ray, Triangle& _outTri, float& _outDist) const {
	float dist;
	if(_ray.Intersects(m_Bounds, dist)) {
		_outDist = FLT_MAX;
		bool intersectFound = false;
		//If this node is a leaf
		if(m_Smallest) {
			Vector3 pos = Vector3(m_Bounds.Center.x, m_Bounds.Center.y, m_Bounds.Center.z);
			//Test all triangles for intersection
			for(unsigned int i = 0; i < m_Polys.size(); i++) {
				Triangle tri = m_Polys[i];
				if(_ray.Intersects(tri.v(0).Position + pos, tri.v(1).Position + pos, tri.v(2).Position + pos, dist) && dist > 0.001f && dist < _outDist) {
					_outTri = tri;
					_outDist = dist;
					intersectFound = true;
				}
			}
		} else {
			//Otherwise test all children
			for(int i = 0; i < m_Children.size(); i++) {
				Triangle tri;
				if(m_Children[i]->Intersect(_ray, tri, dist) && dist > 0.001f && dist < _outDist) {
					_outTri = tri;
					_outDist = dist;
					intersectFound = true;
				}
			}
		}
		return intersectFound;
	}

	return false;
}

bool Node::HasPolys(void) const {
	return m_Polys.size() != 0;
}

Vector3 Node::GetChildPos(int _index) {
	float sizeX = m_Bounds.Extents.x;
	float sizeY = m_Bounds.Extents.y;
	float sizeZ = m_Bounds.Extents.z;

	Vector3 pos = Vector3(m_Bounds.Center.x, m_Bounds.Center.y, m_Bounds.Center.z);

	switch (_index)
	{
	case 0: return (Vector3)(Vector3( sizeX,  sizeY,  sizeZ)/2) + pos; //top	top		top
	case 1: return (Vector3)(Vector3( sizeX,  sizeY, -sizeZ)/2) + pos; //top	top		bottom
	case 2: return (Vector3)(Vector3( sizeX, -sizeY,  sizeZ)/2) + pos; //top	bottom	top
	case 3: return (Vector3)(Vector3( sizeX, -sizeY, -sizeZ)/2) + pos; //top	bottom	bottom
	case 4: return (Vector3)(Vector3(-sizeX,  sizeY,  sizeZ)/2) + pos; //bottom	top		top
	case 5: return (Vector3)(Vector3(-sizeX,  sizeY, -sizeZ)/2) + pos; //bottom	top		bottom
	case 6: return (Vector3)(Vector3(-sizeX, -sizeY,  sizeZ)/2) + pos; //bottom	bottom	top
	case 7: return (Vector3)(Vector3(-sizeX, -sizeY, -sizeZ)/2) + pos; //bottom	bottom	bottom
	}

	return pos;
}

void Node::Devide() {
	for(int i = 0; i < 8; i++) {
		Vector3 extends;
		extends.x = m_Bounds.Extents.x/2;
		extends.y = m_Bounds.Extents.y/2;
		extends.z = m_Bounds.Extents.z/2;

		DirectX::BoundingBox childBounds(GetChildPos(i), extends);
		Node* newChild = new Node(childBounds, m_Polys);

		if(newChild->HasPolys())
			m_Children.push_back(newChild);
	}
}
