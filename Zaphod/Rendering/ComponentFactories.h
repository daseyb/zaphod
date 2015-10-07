#pragma once
#include "PathTracer.h"
#include "BidirectionalPathTracer.h"

inline Integrator* IntegratorFactory(std::string _integrator, Scene* _scene)
{
	if (_integrator == "PT")
	{
		return reinterpret_cast<Integrator*>(new PathTracer(_scene));
	}
	else if (_integrator == "BDPT")
	{
		return reinterpret_cast<Integrator*>(new BidirectionalPathTracer(_scene));
	}

	throw std::invalid_argument(_integrator + " is not a known integrator type.");
}