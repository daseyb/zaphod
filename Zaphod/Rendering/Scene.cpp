#include "Scene.h"
#include "../Objects/BaseObject.h"
#include "../Objects/Sphere.h"
#include "../Objects/Box.h"
#include "../Objects/Mesh.h"
#include "Cameras/Camera.h"
#include "../Light.h"
#include "../DirectionalLight.h"
#include "../PointLight.h"
#include "../LightCache.h"
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include "Materials/Material.h"
#include "Materials/DiffuseMaterial.h"
#include "Materials/SpecularMaterial.h"
#include "Materials/EmissionMaterial.h"

#define USE_LIGHTCACHE 0

using namespace DirectX;
using namespace DirectX::SimpleMath;

Scene::Scene(Camera* _cam)
{
	//Initialize object lists
	m_SceneObjects = std::vector<BaseObject*>();

	//Build a few test objects and materials
	Box* wallFront = new Box(Vector3(0, 10, -8), 20, 20, 0.1f);
	Box* wallBack = new Box(Vector3(0, 10, 12), 20, 20, 0.1f);

	Box* lightBoxFL = new Box(Vector3(5, 2.0f, -5), 1, 1, 1);
	Box* lightBoxFR = new Box(Vector3(-5, 2.0f, -5), 1, 1, 1);

	Box* lightBoxBL = new Box(Vector3(4.5, 2.5f, 5.8), 1, 1, 1);
	Box* lightBoxBR = new Box(Vector3(-4.5, 2.5f, 5.8), 1, 1, 1);

	Box* lightBoxBack = new Box(Vector3(0, 1.0f, 4), 1, 1, 1);

	Sphere* lightSphereRight = new Sphere(0.1f, Vector3(1.85f, 2.6f, 0.05f));
	Sphere* lightSphereLeft = new Sphere(0.1f, Vector3(-1.75f, 2.6f, 0.05f));
	Sphere* lightSphereCenter = new Sphere(0.6f, Vector3(0, 2.6f, 0.05f));

	Box* lightBoxSky = new Box(Vector3(0, 19.0f, -5), 10, 1, 20);
	Box* lightBoxTop = new Box(Vector3(0, 5, 0), 20, 0.1f, 20);

	Box* floor = new Box(Vector3(0, -1.05f, 0), 20, 0.1f, 20);
	Box* wallLeft = new Box(Vector3(-8, 10, 0), 0.1f, 20, 20);
	Box* wallRight = new Box(Vector3(8, 10, 0), 0.1f, 20, 20);

	Mesh* teapot = new Mesh(Vector3(-3.0f, -1.5f, 5), "Data/teapot.obj");
	teapot->SetRotation(Vector3(0, 0, 0));
	teapot->SetScale(Vector3(1, 1, 1));

	Mesh* teddy = new Mesh(Vector3(0, 0, 0), "Data/sponza_merged.obj");
	teddy->SetRotation(Vector3(0, 0, 0));

	Mesh* teddyFront = new Mesh(Vector3(4, -1, 3), "Data/teapot.obj");
	teddyFront->SetRotation(Vector3(-45, 0, 0));

	DiffuseMaterial whiteMat(Color(1.0f, 1.0f, 1.0f));
	EmissionMaterial light(Color(5, 4.5, 4) * 2);
	EmissionMaterial dimLight(Color(3, 2, 1) * 3);


	DiffuseMaterial lightTop(Color(1, 1, 1));
	EmissionMaterial centerLight(Color(0.25f, 0.5f, 1) * 2);
	SpecularMaterial mirror(Color(1, 1, 1), 0.01f, 0.99f, 0.00001f);
	DiffuseMaterial wallMat(Color(0.9f, 0.2f, 0.2f));
	DiffuseMaterial wallMatGreen(Color(0.2f, 0.9f, 0.2f));
	SpecularMaterial floorMat(Color(1.0f, 1.0f, 1.0f), 0.1f, 0.9f, 0.9f);

	SpecularMaterial chromeMatBase(Color(0.5f, 0.5f, 0.5f), 0.4f, 0.6f, 0.3f);
	SpecularMaterial chromeMatRed = chromeMatBase;
	chromeMatRed.DiffuseColor = Color(1.0f, 0.3f, 0.4f);
	SpecularMaterial chromeMatBlue = chromeMatBase;
	chromeMatBlue.DiffuseColor = Color(1.0f, 1.0f, 0.3f);

	lightBoxFL->SetMaterial(&light);
	lightBoxFR->SetMaterial(&light);
	lightBoxBL->SetMaterial(&light);
	lightBoxBR->SetMaterial(&light);
	lightBoxBack->SetMaterial(&light);
	lightSphereRight->SetMaterial(&dimLight);
	lightSphereLeft->SetMaterial(&dimLight);
	lightSphereCenter->SetMaterial(&light);

	EmissionMaterial sky(Color(0.7f, 0.8f, 1) * 13);

	lightBoxSky->SetMaterial(&sky);
	
	lightBoxTop->SetMaterial(&lightTop);
	wallFront->SetMaterial(&whiteMat);
	wallBack->SetMaterial(&whiteMat);
	floor->SetMaterial(&floorMat);
	wallLeft->SetMaterial(&wallMatGreen);
	wallRight->SetMaterial(&wallMat);

	teddy->SetMaterial(&whiteMat);
	teapot->SetMaterial(&whiteMat);
	teddyFront->SetMaterial(&chromeMatRed);

	m_SceneObjects.push_back(teddy);
	m_SceneObjects.push_back(lightBoxFL);
	m_SceneObjects.push_back(lightBoxFR);
	m_SceneObjects.push_back(lightBoxBL);
	m_SceneObjects.push_back(lightBoxBR);
	m_SceneObjects.push_back(lightBoxSky);

	m_SceneObjects.push_back(lightSphereRight);
	m_SceneObjects.push_back(lightSphereLeft);
	//m_SceneObjects.push_back(lightSphereCenter);


	/*m_SceneObjects.push_back(wallFront);
	m_SceneObjects.push_back(wallBack);
	m_SceneObjects.push_back(lightBoxTop);
	m_SceneObjects.push_back(floor);
	m_SceneObjects.push_back(wallLeft);
	m_SceneObjects.push_back(wallRight);
	//m_SceneObjects.push_back(teapot);
	//m_SceneObjects.push_back(teddyFront);*/

	m_SceneLights = std::vector<BaseObject*>();

	m_TotalLightWeight = 0;
	for (auto obj : m_SceneObjects) {
		if (obj->GetMaterial()->IsLight()) {
			m_SceneLights.push_back(obj);
			float weight = obj->CalculateWeight();
			m_LightWeights.push_back(weight);
			m_TotalLightWeight += weight;
		}
	}

	m_SampleDist = std::discrete_distribution<>(
		std::begin(m_LightWeights),
		std::end(m_LightWeights));

	//Set the start time
	m_InitTime = clock();

	//Set the camera pointer and move the camera to it's start position
	m_pCamera = _cam;
	m_pCamera->SetPosition(Vector3(0, 2, 5));
	m_pCamera->SetRotation(0, 0.3, 0);

	m_LightCache = new LightCache(BoundingBox(Vector3(0, 0, 0), Vector3(20, 20, 20)));
}

void Scene::Update()
{
	//Update the time values
	clock_t time = clock();	
	double deltaTime = (double)(time - m_PrevTime)/CLOCKS_PER_SEC;
	double totalTime = (double)(time - m_InitTime)/CLOCKS_PER_SEC;

	m_PrevTime = time;
}

Ray Scene::SampleLight(std::default_random_engine& _rnd, BaseObject** _outLight, float& le) const
{
	assert(m_SceneLights.size() > 0);
	int lightIndex = (int)m_SampleDist(_rnd);
	*_outLight = m_SceneLights[lightIndex];
	le = m_LightWeights[lightIndex] / m_TotalLightWeight;
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
	for(auto obj : m_SceneObjects) {
		delete obj;
	}

	m_SceneObjects.clear();

	m_pCamera = nullptr;
}
