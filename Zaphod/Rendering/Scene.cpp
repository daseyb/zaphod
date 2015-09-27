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
	Sphere* testSphere = new Sphere(1, Vector3(0,0.5f,0));

	//Build a few test objects and materials
	Box* testBox = new Box(Vector3(0, 10, -8), 20, 20, 0.1f);
	Box* ceilingLightBox = new Box(Vector3(0, 6, 0), 3, 0.1f, 3);
	Sphere* lightBox2 = new Sphere(0.3f, Vector3(-3, -1, 4));
	Sphere* lightBox3 = new Sphere(0.5f, Vector3(-3.5f, -1, 5));
	Sphere* lightBox4 = new Sphere(0.2f, Vector3(-2.75, -1, 4.5f));
	
	Box* testBox2 = new Box(Vector3(0, -1.05f, 0), 20, 0.1f, 20);
	Box* testBox3 = new Box(Vector3(-8, 10, 0), 0.1f, 20, 20);
	Box* testBox4 = new Box(Vector3( 8, 10, 0), 0.1f, 20, 20);
	Box* testBox5 = new Box(Vector3(0, 10, 11), 20, 20, 0.1f);
	Mesh* teapot = new Mesh(Vector3(-3.5f, -1.25f, 3), "Data/teapot.obj");
  teapot->SetRotation(Vector3(45, 0, 0));
  Mesh* teddy = new Mesh(Vector3(3, -1, -3), "Data/teddy.obj");
  teddy->SetRotation(Vector3(-45, 0, 0));
  teddy->SetScale(Vector3(2, 2, 2));

	Material whiteMat;
	whiteMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);

	Material light = whiteMat;
	light.DiffuseColor = Color(1, 1, 1);
	light.Emittance = Color(2, 2, 2);

	Material lightTop = whiteMat;
	lightTop.DiffuseColor = Color(1, 1, 1);
	lightTop.Emittance = Color(5, 5, 5);

	Material transparentMat;
	transparentMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);


	Material wallMat;
	wallMat.DiffuseColor = Color(1.0f, 0.1f, 0.1f);

	Material wallMatGreen = wallMat;
	wallMatGreen.DiffuseColor = Color(0.1f, 1.0f, 0.1f);

	Material floorMat;
	floorMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);

	Material chromeMatBase;
	chromeMatBase.DiffuseColor = Color(0.5f, 0.5f, 0.5f);

	Material chromeMatRed = chromeMatBase;
	chromeMatRed.DiffuseColor = Color(1.0f, 0.3f, 1.0f);

	Material chromeMatBlue = chromeMatBase;
	chromeMatBlue.DiffuseColor = Color(1.0f, 1.0f, 0.3f);

	testBox->SetMaterial(whiteMat);
  ceilingLightBox->SetMaterial(lightTop);
	lightBox2->SetMaterial(light);
	lightBox3->SetMaterial(light);
	lightBox4->SetMaterial(light);
	testBox2->SetMaterial(floorMat);
	testBox3->SetMaterial(wallMatGreen);
	testBox4->SetMaterial(wallMat);
	testSphere->SetMaterial(transparentMat);
	teapot->SetMaterial(whiteMat);
	teddy->SetMaterial(chromeMatBlue);

	m_SceneObjects.push_back(teapot);
	m_SceneObjects.push_back(testBox);
  m_SceneObjects.push_back(ceilingLightBox);
	
	m_SceneObjects.push_back(testBox2);
	m_SceneObjects.push_back(testBox3);
	m_SceneObjects.push_back(testBox4);
	m_SceneObjects.push_back(testBox5);
	m_SceneObjects.push_back(teddy);
	
	m_SceneLights = std::vector<Light*>();

	//Set the start time
	m_InitTime = clock();

	//Set the camera pointer and move the camera to it's start position
	m_pCamera = _cam;
	m_pCamera->SetPosition(Vector3(0,2,10));
	m_pCamera->SetRotation(0, -0.3, 0);

	m_LightCache = new LightCache(BoundingBox(Vector3(0, 0, 0), Vector3(20, 20, 20)));
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

Vector3 CosineSampleHemisphere(float u1, float u2)
{
	const float r = sqrt(u1);
	const float theta = 2 * XM_PI * u2;

	const float x = r * cos(theta);
	const float y = r * sin(theta);

	return Vector3(x, y, sqrt(fmax(0.0f, 1.0f - u1)));
}

Vector3 CosWeightedRandomHemisphereDirection2(Vector3 n)
{
	float Xi1 = (float)rand() / (float)RAND_MAX;
	float Xi2 = (float)rand() / (float)RAND_MAX;

	float  theta = acos(sqrt(1.0 - Xi1));
	float  phi = 2.0 * 3.1415926535897932384626433832795 * Xi2;

	float xs = sinf(theta) * cosf(phi);
	float ys = cosf(theta);
	float zs = sinf(theta) * sinf(phi);

	Vector3 y(n.x, n.y, n.z);
	Vector3 h = y;
	if (fabs(h.x) <= fabs(h.y) && fabs(h.x) <= fabs(h.z))
		h.x = 1.0;
	else if (fabs(h.y) <= fabs(h.x) && fabs(h.y) <= fabs(h.z))
		h.y = 1.0;
	else
		h.z = 1.0;


	Vector3 x = (h.Cross(y));
	x.Normalize();
	Vector3 z = (x.Cross(y));
	z.Normalize();

	Vector3 direction = xs * x + ys * y + zs * z;
	direction.Normalize();
	return direction;
}

Vector3 RandomPointOnHemisphere() {
	return CosineSampleHemisphere(GetRnd(), GetRnd());
}

//Intersect a ray with the scene (currently no optimization)
Color Scene::Intersect(const DirectX::SimpleMath::Ray& _ray, int _depth, bool _isSecondary) const
{
  if (_depth == 0) {
    return Color(0, 0, 0);
  }
	
  float minDist = FLT_MAX;
	Intersection minIntersect;
	Intersection intersect;
	bool intersectFound = false;

	//Find the nearest intersection
	for(auto iter = m_SceneObjects.begin(); iter != m_SceneObjects.end(); ++iter)
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


	Color reflected(0, 0, 0);
	
#if USE_LIGHTCACHE
  float rnd = GetRnd();
	int storedCount = 0;
	if (_isSecondary && rnd < 1.0f / (_depth + 1) && m_LightCache->LookUp(minIntersect.position, &reflected, &storedCount) && GetRnd() < storedCount * 0.1f) {
		return reflected;
	} else
#endif



	Color emittance = minIntersect.material.Emittance;

	const int DIFFUSE_SAMPLES = 1;

	for (int i = 0; i < DIFFUSE_SAMPLES; i++)  {
		Vector3 difDir = CosWeightedRandomHemisphereDirection2(minIntersect.normal);
		Ray diffuseRay = Ray(minIntersect.position + difDir * 0.001f, difDir);
		reflected += Intersect(diffuseRay, _depth - 1, true);
	}

	Color BRDF = minIntersect.material.DiffuseColor / DIFFUSE_SAMPLES;
	
	Color final = (emittance + (BRDF * reflected));

#if USE_LIGHTCACHE
	if (storedCount < 10) {
		m_LightCache->AddPoint(minIntersect.position, final);
	}
#endif

	return final ;
}


Scene::~Scene(void)
{
	for(auto iter = m_SceneObjects.begin(); iter != m_SceneObjects.end(); ++iter)
	{
		delete (*iter);
		(*iter) = nullptr;
	}
	m_SceneObjects.clear();

	for(auto iter = m_SceneLights.begin(); iter != m_SceneLights.end(); ++iter)
	{
		delete (*iter);
		(*iter) = nullptr;
	}
	m_SceneLights.clear();

	m_pCamera = nullptr;
}
