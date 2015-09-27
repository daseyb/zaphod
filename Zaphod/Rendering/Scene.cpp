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
	Box* ceilingLightBox = new Box(Vector3(0, 5, 0), 3, 0.1f, 3);
  Sphere *centerLightSphere = new Sphere(0.5f, Vector3(0, -1, 0));

	Box* testBox2 = new Box(Vector3(0, -1.05f, 0), 20, 0.1f, 20);
	Box* testBox3 = new Box(Vector3(-8, 10, 0), 0.1f, 20, 20);
	Box* testBox4 = new Box(Vector3( 8, 10, 0), 0.1f, 20, 20);
	Box* testBox5 = new Box(Vector3(0, 10, 11), 20, 20, 0.1f);

	Mesh* teapot = new Mesh(Vector3(-3.5f, -1.25f, 3), "Data/teapot.obj");
  teapot->SetRotation(Vector3(45, 0, 0));
  
  Mesh* teddy = new Mesh(Vector3(3, -1, -3), "Data/teddy.obj");
  teddy->SetRotation(Vector3(45, 0, 0));

  Mesh* teddyFront = new Mesh(Vector3(4, -1, 3), "Data/teddy.obj");
  teddy->SetRotation(Vector3(-45, 0, 0));

	Material whiteMat;
	whiteMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);

	Material light = whiteMat;
	light.DiffuseColor = Color(1, 1, 1);
	light.Emittance = Color(2, 2, 2);

	Material lightTop = whiteMat;
	lightTop.DiffuseColor = Color(1, 1, 1);
	lightTop.Emittance = Color(0.5f, 0.2f, 0.2f) * 2;

  Material centerLight = lightTop;
  centerLight.Emittance = Color(0.25f, 0.5f, 1) * 2;

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
  centerLightSphere->SetMaterial(centerLight);

	testBox2->SetMaterial(floorMat);
	testBox3->SetMaterial(wallMatGreen);
	testBox4->SetMaterial(wallMat);
	testSphere->SetMaterial(transparentMat);
	teapot->SetMaterial(whiteMat);
	teddy->SetMaterial(chromeMatBlue);
  teddyFront->SetMaterial(chromeMatBlue);

	m_SceneObjects.push_back(teapot);
	m_SceneObjects.push_back(testBox);
  m_SceneObjects.push_back(ceilingLightBox);
  m_SceneObjects.push_back(centerLightSphere);
	
	m_SceneObjects.push_back(testBox2);
	m_SceneObjects.push_back(testBox3);
	m_SceneObjects.push_back(testBox4);
	m_SceneObjects.push_back(testBox5);
	m_SceneObjects.push_back(teddy);
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
	m_pCamera->SetPosition(Vector3(0,2,10));
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

Vector3 CosWeightedRandomHemisphereDirection2(Vector3 n, std::default_random_engine& _rnd)
{
  std::uniform_real_distribution<float> dist = std::uniform_real_distribution<float>(0, 1);
	float Xi1 = dist(_rnd);
	float Xi2 = dist(_rnd);

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


Ray Scene::SampleLight(std::default_random_engine& _rnd, BaseObject** _outLight) const {
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

//Intersect a ray with the scene (currently no optimization)
Color Scene::Intersect(const DirectX::SimpleMath::Ray& _ray, int _depth, bool _isSecondary, std::default_random_engine& _rnd) const
{
  if (_depth == 0) {
    return Color(0, 0, 0);
  }
	
  Intersection minIntersect;
  bool intersectFound = Trace(_ray, minIntersect);

	if(!intersectFound)
	{
		return Color(0,0,0);
	}


	
#if USE_LIGHTCACHE
  float rnd = GetRnd();
	int storedCount = 0;
	if (_isSecondary && rnd < 1.0f / (_depth + 1) && m_LightCache->LookUp(minIntersect.position, &reflected, &storedCount) && GetRnd() < storedCount * 0.1f) {
		return reflected;
	} else
#endif

  Color emittance = minIntersect.material.Emittance;

  Color reflected(0, 0, 0);
  Vector3 difDir; 
  Color BRDF = minIntersect.material.DiffuseColor;

  difDir = CosWeightedRandomHemisphereDirection2(minIntersect.normal, _rnd);
  Ray diffuseRay = Ray(minIntersect.position + difDir * 0.001f, difDir);
  reflected += Intersect(diffuseRay, _depth - 1, true, _rnd) * BRDF;
  


#if 0
  BaseObject* sampledLight;
  auto lightSample = SampleLight(_rnd, &sampledLight);
  difDir = lightSample.position - minIntersect.position;
  float dist = difDir.LengthSquared();
  difDir.Normalize();
  diffuseRay = Ray(minIntersect.position + difDir * 0.001f, difDir);

  float weight = minIntersect.normal.Dot(difDir) * lightSample.direction.Dot(-difDir);
  weight = weight < 0 ? 0 : weight;

  if (weight > 0)
  {
    Intersection lightIntersect;
    bool hitLight = Trace(diffuseRay, lightIntersect);

    if (hitLight && lightIntersect.hitObject == sampledLight)
    {
      weight *= sampledLight->GetWeight();
      weight /= dist;
      reflected = (reflected + lightIntersect.material.Emittance * weight * BRDF)/2;
    }
  }
#endif
	
	Color final = emittance + reflected;

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

	m_pCamera = nullptr;
}
