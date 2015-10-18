#define _USE_MATH_DEFINES
#include "Raytracer.h"
#include "Scene.h"
#include "Cameras/Camera.h"
#include <thread>
#include "Integrators/Integrator.h"
#include "ComponentFactories.h"
#include <iostream>
#include <omp.h>

using namespace DirectX::SimpleMath;

Raytracer::Raytracer(void)
{

}

bool Raytracer::Initialize(int _width, int _height, std::string _integrator, Camera* _camera, int _spp, int _tileSize, int _threads, const char* scene)
{
	std::cout << "Initializing renderer..." << std::endl;

	m_IsShutDown = false;
	m_SPP = _spp;
	m_TileSize = _tileSize;
	m_ThreadCount = _threads;
	m_Width = _width;
	m_Height = _height;

#ifndef HEADLESS
	m_Pixels = new sf::Uint8[m_Width * m_Height * 4] { 0 };
#endif
	
	m_RawPixels = new Color[m_Width * m_Height * 4] { Color(0, 0, 0) };

	m_pCamera.reset(_camera);
	std::cout << "Loading scene..." << std::endl;
	m_pScene = std::make_unique<Scene>(m_pCamera.get(), scene);
	m_pIntegrator.reset(IntegratorFactory(_integrator, m_pScene.get()));

	return true;
}

void Raytracer::Wait() {
	for (int i = 0; i < m_ThreadCount; i++){
		m_Threads[i].join();
	}

	float oneOverSpp = 1.0f / m_SPP;
	for (int i = 0; i < m_Width * m_Height; i++) {
		m_RawPixels[i] *= oneOverSpp;
	}

	m_IsShutDown = true;
}

void Raytracer::Shutdown(void)
{
	if (!m_IsShutDown) {
		m_IsShutDown = true;
		Wait();
	}

#ifndef HEADLESS
	if (m_Pixels)
	{
		delete[] m_Pixels;
		m_Pixels = nullptr;
	}
#endif

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

void Raytracer::RenderPart(int _x, int _y, int _width, int _height)
{
	assert(_x + _width <= m_Width);
	assert(_y + _height <= m_Height);

	std::random_device d;
	std::default_random_engine rnd(d());

	for (int i = 0; i < m_SPP; i++)
	{
		#pragma omp parallel for
		for (int x = _x; x < _x + _width; x++)
		{
			for (int y = _y; y < _y + _height; y++)
			{
				int pixelIndex = (x + m_Width * y) * 4;
				float weight;
				Ray ray = m_pCamera->GetRay(x, y, m_Width, m_Height, rnd, weight);

				Color rayColor = Color(0, 0, 0);

				if (weight > FLT_EPSILON) {
					rayColor = m_pIntegrator->Intersect(ray, BOUNCES, false, rnd) * weight;
				}
				
				Color* pixelAddress = m_RawPixels + x + m_Width *y;
				*pixelAddress += rayColor;

#ifndef HEADLESS
				Color current = m_RawPixels[x + m_Width * y] / (i + 1);
				current.Saturate();

				sf::Color newCol((sf::Uint8)(current.R() * 255), (sf::Uint8)(current.G() * 255), (sf::Uint8)(current.B() * 255), 255);
				memcpy(m_Pixels + pixelIndex, &newCol, 4);
#endif
			}
		}

		if (m_IsShutDown) {
			return;
		}
	}
}

void Raytracer::EmptyQueue(int threadIndex) {
	TileInfo toRender;
	int tileIndex;
	while (!m_IsShutDown) {
		{
			std::lock_guard<std::mutex> lock(m_TileMutex);
			if (m_TilesToRender.size() == 0) {
				break;
			}

			toRender = m_TilesToRender.back();
			m_TilesToRender.pop_back();
			tileIndex = m_TilesToRender.size();
		}
		std::cout << "Rendering tile " << tileIndex << " on thread "<< threadIndex << std::endl;
		RenderPart(toRender.X, toRender.Y, toRender.Width, toRender.Height);
	}
}

void Raytracer::Render(void)
{
	m_pScene->Update();
	
	memset(m_RawPixels, 0, m_Height*m_Width*sizeof(Color));

#ifndef HEADLESS
	memset(m_Pixels, 0, m_Height*m_Width*sizeof(sf::Uint8) * 4);
#endif

#ifdef MULTI_THREADED
	std::cout << "Start rendering..." << std::endl;
	//Spawn rendering threads
	for (int x = 0; x < m_Width; x += m_TileSize) {
		int width = __min(m_TileSize, (m_Width - x));
		for (int y = 0; y < m_Height; y += m_TileSize) {
			int height = __min(m_TileSize, (m_Height - y));
			m_TilesToRender.push_back({x, y, width, height });
		}
	}
	std::cout << "Rendering " << m_TilesToRender.size() << " tiles (" << m_TileSize << ") on " << m_ThreadCount << " threads." << std::endl;

	{
		m_Threads.reserve(m_ThreadCount);
		std::lock_guard<std::mutex> lock(m_TileMutex);
		for (int i = 0; i < m_ThreadCount; i++) {
			m_Threads.push_back(std::thread(&Raytracer::EmptyQueue, this, i));
		}
	}

#else
	RenderPart(0,0,m_Width, m_Height);
#endif

}

#ifndef HEADLESS
sf::Uint8* Raytracer::GetPixels(void) const
{
	return m_Pixels;
}
#endif

Raytracer::~Raytracer(void)
{
	
}
