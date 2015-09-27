#pragma once
#include <SFML/Graphics.hpp>
#include <thread>
#include "../SimpleMath.h"
#include "Camera.h"
#include <mutex>

class Scene;

/********************************************
** Raytracer
** Base class of this renderer, fills an array
** of pixels based on the scenes content
*********************************************/

class Raytracer
{
private:
	static const int THREAD_COUNT = 3;
	
	struct TileInfo {
		int X, Y, Width, Height;
	};

	sf::Uint8* m_Pixels;
	DirectX::SimpleMath::Color* m_RawPixels;
	Camera* m_pCamera;
	Scene* m_pScene;

	float m_FOV;
	int m_Width;
	int m_Height;

	//Get ray for the given X and Y coordinates
	DirectX::SimpleMath::Ray GetRay(int _x, int _y) const;

	std::thread m_Threads[THREAD_COUNT];
	std::vector<TileInfo> m_TilesToRender;

	std::mutex m_TileMutex;


	//Render a part of the image (for multy threading)
	void RenderPart(int _x, int _y, int _width, int _height);

	void EmptyQueue();

	DirectX::SimpleMath::Color ReadColorAt(int _x, int _y) const;


public:
	Raytracer(void);
	bool Initialize(int _width, int _height, float _fov);
	void Shutdown(void);
	void SetFOV(float _fov);

	void Render(void);
	sf::Uint8* GetPixels(void) const;

	~Raytracer(void);
};

