#pragma once
#include "../SimpleMath.h"
#include "Scene.h"
#include "../Geometry/Intersection.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

struct BRDFSample
{
	Vector3 Direction;
	Color Color;
	float PDF;
};

inline Vector3 HemisphereSample(float theta, float phi, Vector3 n)
{
	float xs = sinf(theta) * cosf(phi);
	float ys = cosf(theta);
	float zs = sinf(theta) * sinf(phi);

	Vector3 y(n.x, n.y, n.z);
	Vector3 h = y;

	if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z))
		h.x = 1.0;
	else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z))
		h.y = 1.0;
	else
		h.z = 1.0;


	Vector3 x = (h.Cross(y));
	x.Normalize();
	Vector3 z = (x.Cross(y));
	z.Normalize();

	Vector3 direction = xs * x + ys * y + zs * z;
	direction.Normalize();
	return direction;
}

inline Vector3 CosWeightedRandomHemisphereDirection2(Vector3 n, std::default_random_engine& _rnd)
{
	std::uniform_real_distribution<float> dist = std::uniform_real_distribution<float>(0, 1);

	float Xi1 = (float)dist(_rnd);
	float Xi2 = (float)dist(_rnd);

	float  theta = acos(sqrt(1.0 - Xi1));
	float  phi = 2.0 * XM_PI * Xi2;

	return HemisphereSample(theta, phi, n);
}

inline BRDFSample BRDFDiffuse(const Scene& scene, int currentDepth, Intersection intersect, Vector3 view, std::default_random_engine& _rnd) {
	return{ CosWeightedRandomHemisphereDirection2(intersect.normal, _rnd), intersect.material.DiffuseColor,  1 };
}

inline BRDFSample BRDFPhong(const Scene& scene, int currentDepth, Intersection intersect, Vector3 view, std::default_random_engine& _rnd) {
	std::uniform_real_distribution<float> dist = std::uniform_real_distribution<float>(0, 1);
	float u = dist(_rnd);
	if (u < intersect.material.Kd) {
		return BRDFDiffuse(scene, currentDepth, intersect, view, _rnd);
	}

	float ksd = intersect.material.Kd + intersect.material.Ks;

	Vector3 w1;
	Vector3 ref = Vector3::Reflect(view, -intersect.normal);

	if (intersect.material.Roughness == 0) {
		w1 = ref;
	} else {
		float n = 1.0f / intersect.material.Roughness;

		float u1 = dist(_rnd), u2 = dist(_rnd);
		float theta = acosf(pow(u1, 1.0f / (n + 1)));
		float phi = 2 * XM_PI * u2;

		w1 = HemisphereSample(theta, phi, ref);
	}


	return{ w1, intersect.material.DiffuseColor, 1 };
}