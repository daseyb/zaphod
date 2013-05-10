#define _USE_MATH_DEFINES
#include "Raytracer.h"
#include "Scene.h"
#include <thread>

using namespace DirectX::SimpleMath;

Raytracer::Raytracer(void)
{

}

bool Raytracer::Initialize(int _width, int _height, float _fov)
{
	m_Width = _width;
	m_Height = _height;
	m_FOV = _fov;
	m_Pixels = new sf::Uint8[m_Width * m_Height * 4];
	
	m_pCamera = new Camera();

	m_pScene = new Scene(m_pCamera);

	return true;
}

void Raytracer::Shutdown(void)
{
	if(m_Pixels)
	{
		delete[] m_Pixels;
		m_Pixels = nullptr;
	}

	if(m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = nullptr;
	}

	if(m_pScene)
	{
		delete m_pScene;
		m_pScene = nullptr;
	}
}

void Raytracer::SetFOV(float _fov)
{
	m_FOV = _fov;
}

void Raytracer::RenderPart(int _x, int _y, int _width, int _height)
{
	for(int x = _x; x < _x + _width; x++)
	{
		for (int y = _y; y < _y + _height; y++)
		{
			int pixelIndex = (x + m_Width * y) * 4;
			Ray current = GetRay(x, y);
			Color col = m_pScene->Intersect(current);

			sf::Color newCol((sf::Uint8)(col.R() * 255), (sf::Uint8)(col.G() * 255), (sf::Uint8)(col.B() * 255), 255);
			
			m_Pixels[pixelIndex + 0] = newCol.r;
			m_Pixels[pixelIndex + 1] = newCol.g;
			m_Pixels[pixelIndex + 2] = newCol.b;
			m_Pixels[pixelIndex + 3] = newCol.a;
		}
	}
}

void Raytracer::Render(void)
{
	m_pScene->Update();

	//Spawn rendering threads (hard coded for now)
	const int THREAD_COUNT = 8;

	int width = m_Width/4;
	int height = m_Height/2;

	std::thread t[THREAD_COUNT];

	t[0] = std::thread(&Raytracer::RenderPart, this, 0, 0, width, height);
	t[1] = std::thread(&Raytracer::RenderPart, this, 0, height, width, height);
	t[2] = std::thread(&Raytracer::RenderPart, this, 1 * width, 0*height, width, height);
	t[3] = std::thread(&Raytracer::RenderPart, this, 1 * width, 1*height, width, height);
	t[4] = std::thread(&Raytracer::RenderPart, this, 2 * width, 0*height, width, height);
	t[5] = std::thread(&Raytracer::RenderPart, this, 2 * width, 1*height, width, height);
	t[6] = std::thread(&Raytracer::RenderPart, this, 3 * width, 0*height, width, height);
	t[7] = std::thread(&Raytracer::RenderPart, this, 3 * width, 1*height, width, height);

	for(int i = 0; i < THREAD_COUNT; i++)
		t[i].join();
}

sf::Uint8* Raytracer::GetPixels(void) const
{
	return m_Pixels;
}

Ray Raytracer::GetRay(int _x, int _y) const
{
	float fovx = M_PI * m_FOV / 180; //Horizontal FOV
	float fovy = M_PI * 55 / 180; //Vertical FOV (hard coded to 55)

	float halfWidth = m_Width/2;
	float halfHeight = m_Height/2;

	float alpha = tanf(fovx/2)*((_x - halfWidth)/halfWidth); //horizontal offset
	float beta =  tanf(fovy/2)*((halfHeight - _y)/halfHeight); //vertical offset

	Matrix viewMatrix = m_pCamera->GetViewMatrix();
	Vector3 pos = viewMatrix.Translation();
	Vector3 dir = alpha * viewMatrix.Right() + beta * viewMatrix.Up() + viewMatrix.Forward();
	dir.Normalize();
	
	return Ray(pos, dir);
}

Raytracer::~Raytracer(void)
{
	
}
