#pragma once
#include <SFML/Graphics.hpp>
#include <thread>
#include "../SimpleMath.h"
#include <mutex>
#include <atomic>

class Camera;
class Scene;
class Integrator;

#define MULTI_THREADED
#define BOUNCES 20

/********************************************
** Raytracer
** Base class of this renderer, fills an array
** of pixels based on the scenes content
*********************************************/

class Raytracer
{
private:
	
	struct TileInfo {
		int X, Y, Width, Height;
	};

	sf::Uint8* m_Pixels;
	DirectX::SimpleMath::Color* m_RawPixels;
	
	std::unique_ptr<Camera> m_pCamera;
	std::unique_ptr<Scene> m_pScene;
	std::unique_ptr<Integrator> m_pIntegrator;

	float m_FOV;
	int m_Width;
	int m_Height;
	int m_SPP;
	int m_TileSize;
	int m_ThreadCount;

	std::atomic<bool> m_IsShutDown;

	std::vector<std::thread> m_Threads;
	std::vector<TileInfo> m_TilesToRender;

	std::mutex m_TileMutex;

	//Render a part of the image (for multy threading)
	void RenderPart(int _x, int _y, int _width, int _height);

	void EmptyQueue(int threadIndex);

	DirectX::SimpleMath::Color ReadColorAt(int _x, int _y) const;

public:
	Raytracer(void);
	bool Initialize(int _width, int _height, std::string _integrator, Camera* _camera, int _spp, int _tileSize,  int _threads, const char* scene);
	void Shutdown(void);
	void SetFOV(float _fov);

	void Wait();

	void Render(void);
	sf::Uint8* GetPixels(void) const;
	DirectX::SimpleMath::Color* GetRawPixels(void) const { return m_RawPixels; }

	~Raytracer(void);
};

