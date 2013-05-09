#include "Scene.h"
#include "../Intersection.h"
#include "../Objects/BaseObject.h"
#include "../Objects/Sphere.h"
#include "../Objects/Box.h"
#include "Camera.h"

using namespace DirectX::SimpleMath;

Scene::Scene(Camera* _cam)
{
	sceneObjects = std::vector<BaseObject*>();
	Sphere* testSphere = new Sphere(1, Vector3(0,0,0));
	Box* testBox = new Box(Vector3(0, 0, 0), 1, 1, 1);
	Box* testBox2 = new Box(Vector3(0, -3, 0), 8, 0.1f, 8);
	testBox2->SetDiffuse(Color(1, 0, 0));

	sceneObjects.push_back(testSphere);
	sceneObjects.push_back(testBox);
	sceneObjects.push_back(testBox2);
	initTime = clock();

	m_pCamera = _cam;
	m_pCamera->SetPosition(Vector3(0,0,-10));
}

void Scene::Update()
{
	clock_t time = clock();	
	double deltaTime = (double)(time - prevTime)/CLOCKS_PER_SEC;
	double totalTime = (double)(time - initTime)/CLOCKS_PER_SEC;

	sceneObjects[0]->SetPosition(Vector3(-5 * cosf(totalTime), 0, -5 * sinf(totalTime)));

	//m_pCamera->SetPosition(Vector3(0,0, sinf(totalTime) * 5 + 10));
	m_pCamera->SetRotation(sinf(totalTime*2)/2 + 3.141f,0, 0);

	//m_pCamera->LookAt(Vector3(0, 0, 10), Vector3(0,0,0), Vector3(0,-1,0));

	prevTime = time;
}

Color Scene::Intersect(const DirectX::SimpleMath::Ray& _ray)
{
	float minDist = FLT_MAX;
	Intersection minIntersect;
	Intersection intersect;
	bool intersectFound = false;

	for(auto iter = sceneObjects.begin(); iter != sceneObjects.end(); iter++)
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
	else
	{
		Vector3 lightDir(-1,1,1);
		lightDir.Normalize();

		Ray shadowRay(intersect.position + lightDir * 0.001f, lightDir);
		bool inShadow = false;
		for(auto iter = sceneObjects.begin(); iter != sceneObjects.end(); iter++)
		{
			if((*iter)->Intersect(shadowRay, intersect))
			{
				inShadow = true;
				break;
			}
		}

		float dot = 0;
		if(inShadow)
		{
			dot = 0.15f;
		}
		else
		{
			dot = minIntersect.normal.Dot(lightDir);
			if(dot < 0)
				dot = 0;

			if(dot < 0.15f)
				dot = 0.15f;
		}

		return minIntersect.color * dot;
	}
}


Scene::~Scene(void)
{
	for(auto iter = sceneObjects.begin(); iter != sceneObjects.end(); iter++)
	{
		delete (*iter);
		(*iter) = nullptr;
	}
	sceneObjects.clear();

	m_pCamera = nullptr;
}
