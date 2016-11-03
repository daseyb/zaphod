#include "AlembicCache.h"
#include "../Geometry/Intersection.h"
#include <iterator>



using namespace DirectX::SimpleMath;

AlembicCache::AlembicCache(Vector3 _pos, Vector3 _extends) {
  SetPosition(_pos);
}


void AlembicCache::SetPosition(Vector3 _pos) {
  BaseObject::SetPosition(_pos);
}

float AlembicCache::CalculateWeight() {
  return 0;
}

Ray AlembicCache::Sample(std::default_random_engine &rnd) const {
	return Ray{};
}

bool AlembicCache::Intersect(const Ray &_ray, Intersection &_intersect) const {
  return false;
}

AlembicCache::~AlembicCache(void) {}
