#define _USE_MATH_DEFINES
#include "Raytracer.h"
#include "Scene.h"
#include "Camera.h"
#include <thread>
#include "Integrator.h"
#include "ComponentFactories.h"

using namespace DirectX::SimpleMath;

Raytracer::Raytracer(void)
{

}

bool Raytracer::Initialize(int _width, int _height, std::string _integrator, Camera* _camera)
{
	m_IsShutDown = false;
	m_Width = _width;
	m_Height = _height;
	m_Pixels = new sf::Uint8[m_Width * m_Height * 4] { 0 };
	m_RawPixels = new Color[m_Width * m_Height * 4] { Color(0, 0, 0) };

	m_pCamera.reset(_camera);
	m_pScene = std::make_unique<Scene>(m_pCamera.get());
	m_pIntegrator.reset(IntegratorFactory(_integrator, m_pScene.get()));

	return true;
}

void Raytracer::Shutdown(void)
{
	m_IsShutDown = true;

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
	assert(_x + _width <= m_Width);
	assert(_y + _height <= m_Height);

	std::random_device d;
	std::default_random_engine rnd(d());

	for (int i = 0; i < SAMPLES; i++)
	{
		for (int x = _x; x < _x + _width; x++)
		{
			for (int y = _y; y < _y + _height; y++)
			{
				if (m_IsShutDown) {
					return;
				}

				int pixelIndex = (x + m_Width * y) * 4;
				float weight;
				Ray ray = m_pCamera->GetRay(x, y, m_Width, m_Height, rnd, weight);

				Color rayColor = Color(0, 0, 0);

				if (weight > FLT_EPSILON) {
					rayColor = m_pIntegrator->Intersect(ray, BOUNCES, false, rnd) * weight;
				}

				m_RawPixels[x + m_Width * y] += rayColor;
				Color current = m_RawPixels[x + m_Width * y] / (i + 1);
				current.Saturate();
				/*Color x = current - Color(0.004f, 0.004f, 0.004f);
				x.x = x.x < 0 ? 0 : x.x;
				x.y = x.y < 0 ? 0 : x.y;
				x.z = x.z < 0 ? 0 : x.z;

				current = (x*(6.2f*x + Color(.5f, .5f, .5f))) / (x*(6.2f*x + Color(1.7f, 1.7f, 1.7f)) + Color(0.06f, 0.06f, 0.06f));*/

				sf::Color newCol((sf::Uint8)(current.R() * 255), (sf::Uint8)(current.G() * 255), (sf::Uint8)(current.B() * 255), 255);

				m_Pixels[pixelIndex + 0] = newCol.r;
				m_Pixels[pixelIndex + 1] = newCol.g;
				m_Pixels[pixelIndex + 2] = newCol.b;
				m_Pixels[pixelIndex + 3] = newCol.a;
			}
		}
	}
}

void Raytracer::EmptyQueue() {
	TileInfo toRender;
	while (true) {
		{
			std::lock_guard<std::mutex> lock(m_TileMutex);
			if (m_TilesToRender.size() == 0) {
				break;
			}

			toRender = m_TilesToRender.back();
			m_TilesToRender.pop_back();
		}
		RenderPart(toRender.X, toRender.Y, toRender.Width, toRender.Height);
	}
}

void Raytracer::Render(void)
{
	m_pScene->Update();

#ifdef MULTI_THREADED
	//Spawn rendering threads (hard coded for now)
	for (int x = 0; x < m_Width; x += TILE_SIZE) {
		int width = __min(TILE_SIZE, (m_Width - x));
		for (int y = 0; y < m_Height; y += TILE_SIZE) {
			int height = __min(TILE_SIZE, (m_Height - y));
			m_TilesToRender.push_back({x, y, width, height });
		}
	}

	for (int x = 0; x < m_Width; x++) {
		for (int y = 0; y < m_Height; y++) {
			m_RawPixels[x + m_Width * y] = Color(0, 0, 0);
			int pixelIndex = (x + m_Width * y) * 4;
			m_Pixels[pixelIndex + 0] = 0;
			m_Pixels[pixelIndex + 1] = 0;
			m_Pixels[pixelIndex + 2] = 0;
			m_Pixels[pixelIndex + 3] = 0;
		}
	}

	m_TileMutex.lock();
	for (int i = 0; i < THREAD_COUNT; i++) {
		m_Threads[i] = std::thread(&Raytracer::EmptyQueue, this);
	}
	m_TileMutex.unlock();
#else
	RenderPart(0,0,m_Width, m_Height);
#endif

}

sf::Uint8* Raytracer::GetPixels(void) const
{
	return m_Pixels;
}

Raytracer::~Raytracer(void)
{
	
}
