#pragma once

#include "Vertex.h"

class Line
{
public:
	Vertex a;
	Vertex b;

	Line(Vertex _a, Vertex _b) : a(_a), b(_b) {}
	~Line(void);

};

