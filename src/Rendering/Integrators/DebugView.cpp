#include "DebugView.h"
#include "../../Geometry/Intersection.h"
#include "../Scene.h"
#include "../BRDFs.h"
#include "../Materials/Material.h"
#include "../../Objects/BaseObject.h"
#include "../../Objects/Sphere.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

#define RUSSIAN_ROULETTE 1.0f

// Intersect a ray with the scene (currently no optimization)
Color DebugView::Intersect(const Ray &_ray, int _depth, bool _isSecondary,
                            std::default_random_engine &_rnd) const {
  if (_depth == 0) {
    return Color(0, 0, 0);
  }

  Color weight = Color(1, 1, 1);
  weight.A(0);
  Color L = Color(0, 0, 0, 0);

  Ray currentRay = _ray;
  Intersection minIntersect;
  bool intersectFound = m_Scene->Trace(currentRay, minIntersect);

  if (!intersectFound) {
		return L;
  }

	return Color(minIntersect.normal * 0.5 + Vector3{ 0.5, 0.5, 0.5 });
}
