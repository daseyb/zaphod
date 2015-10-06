#include "Scene.h"
#include "../Geometry/Intersection.h"
#include "../Objects/BaseObject.h"
#include "../Objects/Sphere.h"
#include "../Objects/Box.h"
#include "../Objects/Mesh.h"
#include "Camera.h"
#include "../Light.h"
#include "../DirectionalLight.h"
#include "../PointLight.h"
#include "../LightCache.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>

#define USE_LIGHTCACHE 0

using namespace DirectX;
using namespace DirectX::SimpleMath;

Scene::Scene(Camera* _cam)
{
	//Initialize object lists
	m_SceneObjects = std::vector<BaseObject*>();
	Sphere* testSphere = new Sphere(1, Vector3(0, 0.5f, 0));

	//Build a few test objects and materials
	Box* wallFront = new Box(Vector3(0, 10, -8), 20, 20, 0.1f);
	Box* wallBack = new Box(Vector3(0, 10, 12), 20, 20, 0.1f);

	Box* lightBox = new Box(Vector3(0, 4.5f, 0), 1, 0.1f, 1);
	Box* lightBoxTop = new Box(Vector3(0, 5, 0), 20, 0.1f, 20);

	Box* floor = new Box(Vector3(0, -1.05f, 0), 20, 0.1f, 20);
	Box* wallLeft = new Box(Vector3(-8, 10, 0), 0.1f, 20, 20);
	Box* wallRight = new Box(Vector3(8, 10, 0), 0.1f, 20, 20);

	Mesh* teapot = new Mesh(Vector3(-3.5f, 0, -4), "Data/test_smooth.obj");
	teapot->SetRotation(Vector3(45, 0, -45));
	teapot->SetScale(Vector3(3, 3, 3));

	Mesh* teddy = new Mesh(Vector3(3, 0, -3), "Data/test_smooth.obj");
	teddy->SetRotation(Vector3(45, 0, 0));

	Mesh* teddyFront = new Mesh(Vector3(4, 0, 3), "Data/test_smooth.obj");
	teddyFront->SetRotation(Vector3(-45, 0, 0));

	Material whiteMat;
	whiteMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);

	Material light = whiteMat;
	light.DiffuseColor = Color(1, 1, 1);
	light.Emittance = Color(10, 10, 10);

	Material lightTop = whiteMat;
	lightTop.DiffuseColor = Color(1, 1, 1);

	Material centerLight = lightTop;
	centerLight.Emittance = Color(0.25f, 0.5f, 1) * 2;

	Material mirror = { Color(1, 1, 1), Color(0, 0, 0), 0.01f, 0.99f, 0.00001f };

	Material transparentMat;
	transparentMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);

	Material wallMat;
	wallMat.DiffuseColor = Color(0.9f, 0.2f, 0.2f);

	Material wallMatGreen = wallMat;
	wallMatGreen.DiffuseColor = Color(0.2f, 0.9f, 0.2f);

	Material wallMatBlue = wallMat;
	wallMatBlue.DiffuseColor = Color(0.2f, 0.2f, 0.9f);

	Material floorMat;
	floorMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);
	floorMat.Roughness = 0.9f;
	floorMat.Kd = 0.1f;
	floorMat.Ks = 0.9f;

	Material chromeMatBase;
	chromeMatBase.DiffuseColor = Color(0.5f, 0.5f, 0.5f);
	chromeMatBase.Roughness = 0.3f;
	chromeMatBase.Kd = 0.4f;
	chromeMatBase.Ks = 0.6f;

	Material chromeMatRed = chromeMatBase;
	chromeMatRed.DiffuseColor = Color(1.0f, 0.3f, 1.0f);

	Material chromeMatBlue = chromeMatBase;
	chromeMatBlue.DiffuseColor = Color(1.0f, 1.0f, 0.3f);

	lightBox->SetMaterial(light);
	lightBoxTop->SetMaterial(lightTop);
	wallFront->SetMaterial(mirror);
	wallBack->SetMaterial(mirror);
	floor->SetMaterial(floorMat);
	wallLeft->SetMaterial(wallMatGreen);
	wallRight->SetMaterial(wallMat);
	testSphere->SetMaterial(transparentMat);

	teddy->SetMaterial(chromeMatBlue);
	teapot->SetMaterial(chromeMatRed);
	teddyFront->SetMaterial(chromeMatRed);

	m_SceneObjects.push_back(wallFront);
	m_SceneObjects.push_back(wallBack);
	m_SceneObjects.push_back(lightBox);
	m_SceneObjects.push_back(lightBoxTop);
	m_SceneObjects.push_back(floor);
	m_SceneObjects.push_back(wallLeft);
	m_SceneObjects.push_back(wallRight);
	m_SceneObjects.push_back(teddy);
	m_SceneObjects.push_back(teapot);
	m_SceneObjects.push_back(teddyFront);

	m_SceneLights = std::vector<BaseObject*>();

	for (auto obj : m_SceneObjects) {
		float emmittanceMagnitude = obj->GetMaterial().Emittance.ToVector3().LengthSquared();
		if (emmittanceMagnitude > 0.1f) {
			m_SceneLights.push_back(obj);
			m_LightWeights.push_back(obj->CalculateWeight() + emmittanceMagnitude);
		}
	}

	m_SampleDist = std::discrete_distribution<>(
		std::begin(m_LightWeights),
		std::end(m_LightWeights));


	//Set the start time
	m_InitTime = clock();

	//Set the camera pointer and move the camera to it's start position
	m_pCamera = _cam;
	m_pCamera->SetPosition(Vector3(0, 2, 10));
	m_pCamera->SetRotation(0, -0.3, 0);

	m_LightCache = new LightCache(BoundingBox(Vector3(0, 0, 0), Vector3(20, 20, 20)));
	srand(time(NULL));
	m_Rnd = std::default_random_engine();
}

void Scene::Update()
{
	//Update the time values
	clock_t time = clock();	
	double deltaTime = (double)(time - m_PrevTime)/CLOCKS_PER_SEC;
	double totalTime = (double)(time - m_InitTime)/CLOCKS_PER_SEC;

	m_PrevTime = time;
}

Ray Scene::SampleLight(std::default_random_engine& _rnd, BaseObject** _outLight) const
{
  int lightIndex = (int)m_SampleDist(_rnd);
  *_outLight = m_SceneLights[lightIndex];
  return (*_outLight)->Sample(_rnd);
}

bool Scene::Trace(const DirectX::SimpleMath::Ray& _ray, Intersection& minIntersect) const
{
	float minDist = FLT_MAX;
	Intersection intersect;
	bool intersectFound = false;

	//Find the nearest intersection
	for (auto obj : m_SceneObjects)
	{
		if (obj->Intersect(_ray, intersect))
		{
			float dist = (intersect.position - _ray.position).LengthSquared();
			if (dist < minDist)
			{
				minDist = dist;
				intersectFound = true;
				minIntersect = intersect;
				minIntersect.hitObject = obj;
			}
		}
	}

	return intersectFound;
}

Scene::~Scene(void)
{
	for(auto iter = m_SceneObjects.begin(); iter != m_SceneObjects.end(); ++iter)
	{
		delete (*iter);
		(*iter) = nullptr;
	}
	m_SceneObjects.clear();

	m_pCamera = nullptr;
}
