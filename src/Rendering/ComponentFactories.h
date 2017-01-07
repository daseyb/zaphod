#pragma once
#include "Integrators/PathTracer.h"
#include "Integrators/BidirectionalPathTracer.h"
#include "Integrators/GradientDomainPathTracer.h"
#include "Integrators/DebugView.h"

inline Integrator *IntegratorFactory(std::string _integrator, Scene *_scene, Camera* _camera, int w, int h) {
  if (_integrator == "PT") {
    return reinterpret_cast<Integrator *>(new PathTracer(_scene, _camera, w, h));
  } else if (_integrator == "BDPT") {
    return reinterpret_cast<Integrator *>(new BidirectionalPathTracer(_scene, _camera, w, h));
  } else if (_integrator == "GDPT") {
    return reinterpret_cast<Integrator *>(new GradientDomainPathTracer(_scene, _camera, w, h));
  } else if (_integrator == "DV") {
	return reinterpret_cast<Integrator *>(new DebugView(_scene, _camera, w, h));
}

  throw std::invalid_argument(_integrator + " is not a known integrator type.");
}