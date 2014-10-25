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
#include <stdlib.h>
#include <iostream>

using namespace DirectX::SimpleMath;

Scene::Scene(Camera* _cam)
{
	//Initialize object lists
	m_SceneObjects = std::vector<BaseObject*>();
	Sphere* testSphere = new Sphere(1, Vector3(0,0.5f,0));

	Sphere* testSphere2 = new Sphere(2.5f, Vector3(3.5f,0.5f,2));
	Sphere* testSphere3 = new Sphere(2.5f, Vector3(-3.5f,0.5f,-1));

	//Build a few test objects and materials
	Box* testBox = new Box(Vector3(0, 10, -8), 20, 20, 0.1f);
	Box* testBox2 = new Box(Vector3(0, -1.05f, 0), 20, 0.1f, 20);
	Box* testBox3 = new Box(Vector3(-8, 10, 0), 0.1f, 20, 20);
	Box* testBox4 = new Box(Vector3( 8, 10, 0), 0.1f, 20, 20);
	Box* testBox5 = new Box(Vector3(0, 10, 11), 20, 20, 0.1f);
	Mesh* monkey = new Mesh(Vector3(0, 0, 0), "Data/CornellBox-Original.obj");

	Material whiteMat;
	whiteMat.DiffuseColor = Color(0.0f, 0.0f, 0.0f);
	whiteMat.Emittance = Color(2.0f, 2.0f, 2.0f);

	Material transparentMat;
	transparentMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);

	testBox->SetMaterial(whiteMat);

	Material wallMat;
	wallMat.DiffuseColor = Color(1.0f, 0.1f, 0.1f);

	Material wallMatGreen = wallMat;
	wallMatGreen.DiffuseColor = Color(0.1f, 1.0f, 0.1f);

	Material floorMat;
	floorMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);

	Material chromeMatBase;
	chromeMatBase.DiffuseColor = Color(0.5f, 0.5f, 0.5f);

	Material chromeMatRed = chromeMatBase;
	chromeMatRed.DiffuseColor = Color(1.0f, 1.0f, 1.0f);
	chromeMatRed.Roughness = 0;

	Material chromeMatBlue = chromeMatBase;
	chromeMatBlue.DiffuseColor = Color(1.0f, 1.0f, 1.0f);

	testBox2->SetMaterial(floorMat);
	testBox3->SetMaterial(wallMatGreen);
	testBox4->SetMaterial(wallMat);
	testSphere->SetMaterial(transparentMat);
	monkey->SetMaterial(whiteMat);
	testSphere2->SetMaterial(chromeMatBlue);
	testSphere3->SetMaterial(chromeMatRed);

	//m_SceneObjects.push_back(monkey);
	m_SceneObjects.push_back(testBox);
	//m_SceneObjects.push_back(testSphere);
	m_SceneObjects.push_back(testBox2);
	m_SceneObjects.push_back(testBox3);
	m_SceneObjects.push_back(testBox4);
	m_SceneObjects.push_back(testBox5);
	m_SceneObjects.push_back(testSphere2);
	m_SceneObjects.push_back(testSphere3);
	
	//Build a few test lights
	PointLight* pointTest = new PointLight(Color(1.0, 1.0f, 1.0f), 1.9f, Vector3(0, 2, -5), 15);
	PointLight* pointTest2 = new PointLight(Color(0.2f, 1.0f, 0.2f), 0.9f, Vector3(-2, 2, 2), 5);
	PointLight* pointTest3 = new PointLight(Color(0.0f, 0.66f, 1.0f), 2.5f, Vector3(0, 1.9f, 5), 10);

	DirectionalLight* dirTest = new DirectionalLight(Color(1, 1, 1), 0.2f, Vector3(-1, -1, -2));

	m_SceneLights = std::vector<Light*>();
	//m_SceneLights.push_back(pointTest2);
	m_SceneLights.push_back(pointTest);
	//m_SceneLights.push_back(pointTest3);
	//m_SceneLights.push_back(dirTest);


	//Set the start time
	m_InitTime = clock();

	//Set the camera pointer and move the camera to it's start position
	m_pCamera = _cam;
	m_pCamera->SetPosition(Vector3(0,2,10));
	m_pCamera->SetRotation(0, -0.3, 0);

	srand(time(NULL));
}

void Scene::Update()
{
	//Update the time values
	clock_t time = clock();	
	double deltaTime = (double)(time - m_PrevTime)/CLOCKS_PER_SEC;
	double totalTime = (double)(time - m_InitTime)/CLOCKS_PER_SEC;

	m_PrevTime = time;
}

float GetRnd()
{
	return (float)rand()/RAND_MAX;
}

Vector3 RandomOffset(Vector3 _dir, float _factor) {
	return _dir + Vector3((GetRnd() - 0.5f) * 0.2f * _factor, (GetRnd() - 0.5f) * 0.2f * _factor, (GetRnd() - 0.5f) * 0.2f * _factor);
}

Vector3 CosineSampleHemisphere(float u1, float u2)
{
	const float r = sqrt(u1);
	const float theta = 2 * 3.141592653 * u2;

	const float x = r * cos(theta);
	const float y = r * sin(theta);

	return Vector3(x, y, sqrt(fmax(0.0f, 1 - u1)));
}

Vector3 RandomPointOnHemisphere() {
	return CosineSampleHemisphere(GetRnd(), GetRnd());
}

//Intersect a ray with the scene (currently no optimization)
Color Scene::Intersect(const DirectX::SimpleMath::Ray& _ray, int _depth) const
{
	if(_depth < 0)
		return Color(0,0,0);

	float minDist = FLT_MAX;
	Intersection minIntersect;
	Intersection intersect;
	bool intersectFound = false;

	//Find the nearest intersection
	for(auto iter = m_SceneObjects.begin(); iter != m_SceneObjects.end(); iter++)
	{
		if((*iter)->Intersect(_ray, intersect))
		{
			float dist = (intersect.position - _ray.position).LengthSquared();
			if(dist < minDist)
			{
				minDist = dist;
				intersectFound = true;
				minIntersect = intersect;
			}
		}
	}

	if(!intersectFound)
	{
		return Color(0,0,0);
	}

	Color emittance = minIntersect.material.Emittance;
	Vector3 dir = RandomPointOnHemisphere();
	Vector3 offset = dir - Vector3(0, 0, 1);
	Vector3 rayDir = minIntersect.normal + offset * minIntersect.material.Roughness;
	rayDir.Normalize();
	Ray diffuseRay = Ray(minIntersect.position + rayDir * 0.001f, rayDir);
	Color reflected = Intersect(diffuseRay, _depth - 1);

	float cos_theta = rayDir.Dot(minIntersect.normal);
	Color BRDF = 2 * minIntersect.material.DiffuseColor;
	
	Color final = emittance + (BRDF * reflected);
	return final;
}


Scene::~Scene(void)
{
	for(auto iter = m_SceneObjects.begin(); iter != m_SceneObjects.end(); iter++)
	{
		delete (*iter);
		(*iter) = nullptr;
	}
	m_SceneObjects.clear();

	for(auto iter = m_SceneLights.begin(); iter != m_SceneLights.end(); iter++)
	{
		delete (*iter);
		(*iter) = nullptr;
	}
	m_SceneLights.clear();

	m_pCamera = nullptr;
}
