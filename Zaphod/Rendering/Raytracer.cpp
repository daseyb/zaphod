#define _USE_MATH_DEFINES
#include "Raytracer.h"
#include "Scene.h"
#include <thread>

using namespace DirectX::SimpleMath;

#define MULTI_THREADED

Raytracer::Raytracer(void)
{

}

bool Raytracer::Initialize(int _width, int _height, float _fov)
{
	m_Width = _width;
	m_Height = _height;
	m_FOV = _fov;
	m_Pixels = new sf::Uint8[m_Width * m_Height * 4];
	m_RawPixels = new Color[m_Width * m_Height * 4];

	m_pCamera = new Camera();

	m_pScene = new Scene(m_pCamera);

	return true;
}

void Raytracer::Shutdown(void)
{
	for (int i = 0; i < THREAD_COUNT; i++)
		m_Threads[i].join();

	if(m_Pixels)
	{
		delete[] m_Pixels;
		m_Pixels = nullptr;
	}

	if (m_RawPixels)
	{
		delete[] m_RawPixels;
		m_RawPixels = nullptr;
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

Color Raytracer::ReadColorAt(int _x, int _y) const
{
	int pixelIndex = (_x + m_Width * _y) * 4;
	return Color(m_Pixels[pixelIndex + 0], m_Pixels[pixelIndex + 1], m_Pixels[pixelIndex + 2], m_Pixels[pixelIndex + 3]) * 1.0f / 255;
}

void Raytracer::RenderPart(int _x, int _y, int _width, int _height)
{
	for (int i = 0; i < 3000; i++)
	{
		for (int x = _x; x < _x + _width; x++)
		{
			for (int y = _y; y < _y + _height; y++)
			{
				int pixelIndex = (x + m_Width * y) * 4;
				if (i == 0) {
					m_RawPixels[x + m_Width * y] = Color(0, 0, 0);
 				}

				Ray ray = GetRay(x, y);

				Color rayColor = m_pScene->Intersect(ray, 4);

				m_RawPixels[x + m_Width * y] += rayColor;
				Color current = m_RawPixels[x + m_Width * y] / (i+1);
				current.Saturate();

				sf::Color newCol((sf::Uint8)(current.R() * 255), (sf::Uint8)(current.G() * 255), (sf::Uint8)(current.B() * 255), 255);

				m_Pixels[pixelIndex + 0] = newCol.r;
				m_Pixels[pixelIndex + 1] = newCol.g;
				m_Pixels[pixelIndex + 2] = newCol.b;
				m_Pixels[pixelIndex + 3] = newCol.a;
			}
		}
	}

}

void Raytracer::Render(void)
{
	m_pScene->Update();

#ifdef MULTI_THREADED
	//Spawn rendering threads (hard coded for now)
	int width = m_Width/4;
	int height = m_Height/2;

	m_Threads[0] = std::thread(&Raytracer::RenderPart, this, 0, 0, width, height);
	m_Threads[1] = std::thread(&Raytracer::RenderPart, this, 0, height, width, height);
	m_Threads[2] = std::thread(&Raytracer::RenderPart, this, 1 * width, 0 * height, width, height);
	m_Threads[3] = std::thread(&Raytracer::RenderPart, this, 1 * width, 1 * height, width, height);
	m_Threads[4] = std::thread(&Raytracer::RenderPart, this, 2 * width, 0 * height, width, height);
	m_Threads[5] = std::thread(&Raytracer::RenderPart, this, 2 * width, 1 * height, width, height);
	m_Threads[6] = std::thread(&Raytracer::RenderPart, this, 3 * width, 0 * height, width, height);
	m_Threads[7] = std::thread(&Raytracer::RenderPart, this, 3 * width, 1 * height, width, height);
#else
	RenderPart(0,0,m_Width, m_Height);
#endif

}

sf::Uint8* Raytracer::GetPixels(void) const
{
	return m_Pixels;
}

Ray Raytracer::GetRay(int _x, int _y) const
{
	float x = _x + ((float)rand() / RAND_MAX - 0.5f) * 2;
	float y = _y + ((float)rand() / RAND_MAX - 0.5f) * 2;
	float fovx =  M_PI * m_FOV / 180; //Horizontal FOV
	float fovy =  M_PI * 55 / 180; //Vertical FOV (hard coded to 55)

	float halfWidth = m_Width/2;
	float halfHeight = m_Height/2;

	float alpha = tanf(fovx / 2)*((x - halfWidth) / halfWidth);
	float beta = tanf(fovy / 2)*((halfHeight - y) / halfHeight);

	Matrix viewMatrix = m_pCamera->GetViewMatrix();
	Vector3 pos = viewMatrix.Translation();
	Vector3 dir = alpha * viewMatrix.Right() + beta * viewMatrix.Up() + viewMatrix.Forward();
	dir.Normalize();
	
	return Ray(pos, dir);
}

Raytracer::~Raytracer(void)
{
	
}
