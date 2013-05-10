#pragma once
#include <SFML/Graphics.hpp>
#include "../SimpleMath.h"
#include "Camera.h"

class Scene;

/********************************************
** Raytracer
** Base class of this renderer, fills an array
** of pixels based on the scenes content
*********************************************/

class Raytracer
{
private:
	sf::Uint8* m_Pixels;
	Camera* m_pCamera;
	Scene* m_pScene;

	float m_FOV;
	int m_Width;
	int m_Height;

	//Get ray for the given X and Y coordinates
	DirectX::SimpleMath::Ray GetRay(int _x, int _y) const;

	//Render a part of the image (for multy threading)
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

