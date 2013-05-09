#pragma once
#include <SFML/Graphics.hpp>
#include "SimpleMath.h"
#include "Camera.h"

class Scene;

class Raytracer
{
private:
	sf::Uint8* m_Pixels;
	Camera* m_pCamera;
	Scene* m_pScene;

	float m_FOV;
	int m_Width;
	int m_Height;

	DirectX::SimpleMath::Ray GetRay(int _x, int _y) const;
	void RenderPart(int _x, int _y, int _width, int _height);

public:
	Raytracer(void);
	bool Initialize(int _width, int _height, float _fov);
	void Shutdown(void);
	void SetFOV(float _fov);

	void Render(void);
	sf::Uint8* GetPixels(void) const;

	~Raytracer(void);
};

