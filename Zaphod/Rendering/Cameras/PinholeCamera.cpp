#define _USE_MATH_DEFINES
#include "PinholeCamera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Ray PinholeCamera::GetRay(int _x, int _y, int _w, int _h, std::default_random_engine & _rnd, float& weight) const
{
	std::uniform_real_distribution<float> dist(-1, 1);

	float x = _x + dist(_rnd);
	float y = _y + dist(_rnd);
	float fovx = M_PI * m_FOV / 180; //Horizontal FOV
	float fovy = M_PI * 55 / 180; //Vertical FOV (hard coded to 55)

	float halfWidth = _w / 2;
	float halfHeight = _h / 2;

	float alpha = tanf(fovx / 2)*((x - halfWidth) / halfWidth);
	float beta = tanf(fovy / 2)*((halfHeight - y) / halfHeight);

	Matrix viewMatrix = GetViewMatrix();
	Vector3 pos = viewMatrix.Translation();
	Vector3 dir = alpha * viewMatrix.Right() + beta * viewMatrix.Up() + viewMatrix.Forward();
	dir.Normalize();

	weight = 1;
	return Ray(pos, dir);
}
