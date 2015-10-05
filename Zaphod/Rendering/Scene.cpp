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

using namespace DirectX;
using namespace DirectX::SimpleMath;

Scene::Scene(Camera* _cam)
{
	//Initialize object lists
	m_SceneObjects = std::vector<BaseObject*>();
	Sphere* testSphere = new Sphere(1, Vector3(0,0.5f,0));

	Sphere* testSphere2 = new Sphere(2.5f, Vector3(3.5f,0.5f,2));
	Sphere* testSphere3 = new Sphere(2.5f, Vector3(-3.5f,0.5f,-1));

	//Build a few test objects and materials
	Box* wallFront = new Box(Vector3(0, 10, -8), 20, 20, 0.1f);
	Box* wallBack = new Box(Vector3(0, 10, 12), 20, 20, 0.1f);

	Box* lightBox = new Box(Vector3(0, 3, -7), 3, 3, 0.1f);
	Sphere* lightBox2 = new Sphere(0.3f, Vector3(-3, -1, 4));
	Sphere* lightBox3 = new Sphere(0.5f, Vector3(-3.5f, -1, 5));
	Sphere* lightBox4 = new Sphere(0.2f, Vector3(-2.75, -1, 4.5f));
	
	Box* lightBoxTop = new Box(Vector3(0, 5, 0), 20, 0.1f, 20);

	Box* floor = new Box(Vector3(0, -1.05f, 0), 20, 0.1f, 20);
	Box* wallLeft = new Box(Vector3(-8, 10, 0), 0.1f, 20, 20);
	Box* wallRight = new Box(Vector3(8, 10, 0), 0.1f, 20, 20);
	//Mesh* monkey = new Mesh(Vector3(-3.5f, 0.5f, 4), "Data/test_smooth.obj");

	Material whiteMat;
	whiteMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);

	Material light = whiteMat;
	light.DiffuseColor = Color(1, 1, 1);
	light.Emittance = Color(1, 1, 1);

	Material lightTop = whiteMat;
	lightTop.DiffuseColor = Color(1, 1, 1);
	lightTop.Emittance = Color(0.0f, 0.0f, 0.0f);

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
	floorMat.Roughness = 0.025f;
	floorMat.Kd = 0.1f;
	floorMat.Ks = 0.9f;

	Material chromeMatBase;
	chromeMatBase.DiffuseColor = Color(0.5f, 0.5f, 0.5f);
	chromeMatBase.Roughness = 1;
	chromeMatBase.Kd = 1.0f;
	chromeMatBase.Ks = 0.0f;

	Material chromeMatRed = chromeMatBase;
	chromeMatRed.DiffuseColor = Color(1.0f, 0.3f, 1.0f);

	Material chromeMatBlue = chromeMatBase;
	chromeMatBlue.DiffuseColor = Color(1.0f, 1.0f, 0.3f);

	lightBox->SetMaterial(light);
	lightBox2->SetMaterial(light);
	lightBox3->SetMaterial(light);
	lightBox4->SetMaterial(light);
	lightBoxTop->SetMaterial(lightTop);

	wallFront->SetMaterial(whiteMat);
	wallBack->SetMaterial(wallMatBlue);

	floor->SetMaterial(floorMat);
	wallLeft->SetMaterial(wallMatGreen);
	wallRight->SetMaterial(wallMat);
	testSphere->SetMaterial(transparentMat);
	//monkey->SetMaterial(whiteMat);
	testSphere2->SetMaterial(chromeMatBlue);
	testSphere3->SetMaterial(chromeMatRed);

	//m_SceneObjects.push_back(monkey);
	m_SceneObjects.push_back(wallFront);
	m_SceneObjects.push_back(wallBack);
	m_SceneObjects.push_back(lightBox);
	m_SceneObjects.push_back(lightBox2);
	m_SceneObjects.push_back(lightBox3);
	m_SceneObjects.push_back(lightBox4);
	m_SceneObjects.push_back(lightBoxTop);
	m_SceneObjects.push_back(floor);
	m_SceneObjects.push_back(wallLeft);
	m_SceneObjects.push_back(wallRight);
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

Vector3 HemisphereSample(float theta, float phi, Vector3 n) {
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

Vector3 CosWeightedRandomHemisphereDirection2(Vector3 n)
{
	float Xi1 = (float)rand() / (float)RAND_MAX;
	float Xi2 = (float)rand() / (float)RAND_MAX;

	float  theta = acos(sqrt(1.0 - Xi1));
	float  phi = 2.0 * 3.1415926535897932384626433832795 * Xi2;

	return HemisphereSample(theta, phi, n);
}

Color BRDFDiffuse(const Scene& scene, int currentDepth, Intersection intersect, Vector3 view) {
	Vector3 difDir = CosWeightedRandomHemisphereDirection2(intersect.normal);
	Ray diffuseRay = Ray(intersect.position + difDir * 0.001f, difDir);
	return scene.Intersect(diffuseRay, currentDepth - 1, true) * XM_PI;
}

Color BRDFPhong(const Scene& scene, int currentDepth, Intersection intersect, Vector3 view) {
	float u = GetRnd();
	if (u < intersect.material.Kd) {
		return BRDFDiffuse(scene, currentDepth, intersect, view);
	}

	float ksd = intersect.material.Kd + intersect.material.Ks;
	if (ksd < u) {
		return Color(0, 0, 0);
	}

	Vector3 w1;
	Vector3 ref = Vector3::Reflect(view, -intersect.normal);

	if (intersect.material.Roughness == 0) {
		w1 = ref;
	} else {
		float n = 1.0f / intersect.material.Roughness;

		float u1 = GetRnd(), u2 = GetRnd();
		float theta = acosf(pow(u1, 1.0f / (n + 1)));
		float phi = 2 * XM_PI * u2;

		w1 = HemisphereSample(theta, phi, ref);
	}


	Ray specRay = Ray(intersect.position + w1 * 0.001f, w1);
	return scene.Intersect(specRay, currentDepth - 1, true);
}



//Intersect a ray with the scene (currently no optimization)
Color Scene::Intersect(const DirectX::SimpleMath::Ray& _ray, int _depth, bool _isSecondary) const
{
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


	Color reflected(0, 0, 0);
	float rnd = GetRnd();
	int storedCount = 0;
	
	/*if (_isSecondary && rnd < 1.0f / (_depth + 1) && m_LightCache->LookUp(minIntersect.position, &reflected, &storedCount) && GetRnd() < storedCount * 0.1f) {
		return reflected;
	} else*/ if (_depth == 0) {
		return Color(0, 0, 0);
	}

	Color emittance = minIntersect.material.Emittance;

	const int DIFFUSE_SAMPLES = 1;

	for (int i = 0; i < DIFFUSE_SAMPLES; i++)  {
		reflected += BRDFPhong(*this, _depth, minIntersect, _ray.direction);
	}

	Color BRDF = minIntersect.material.DiffuseColor / DIFFUSE_SAMPLES;
	
	Color final = (emittance + (BRDF * reflected));

	/*if (storedCount < 10) {
		m_LightCache->AddPoint(minIntersect.position, final);
	}*/

	return final ;
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
