#include "Triangle.h"


Triangle::Triangle() {

}

Triangle::Triangle(Vertex _v1, Vertex _v2, Vertex _v3) {
	m_Vertices[0] = _v1;
	m_Vertices[1] = _v2;
	m_Vertices[2] = _v3;
}

bool Triangle::operator==(const Triangle& rhs) const {
	return m_Vertices[0] == rhs.m_Vertices[0] && m_Vertices[1] == rhs.m_Vertices[1] && m_Vertices[2] == rhs.m_Vertices[2];
}

Vertex Triangle::v(int _index) const {
	if(_index < 0 || _index > 2)
		return Vertex();
	return m_Vertices[_index];
}


Triangle::~Triangle(void) {
}
