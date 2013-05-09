#include "Scene.h"
#include "../Intersection.h"
#include "../Objects/BaseObject.h"
#include "../Objects/Sphere.h"
#include "../Objects/Box.h"
#include "Camera.h"
#include "../Light.h"
#include "../DirectionalLight.h"
#include "../PointLight.h"

using namespace DirectX::SimpleMath;

Scene::Scene(Camera* _cam)
{
	m_SceneObjects = std::vector<BaseObject*>();
	Sphere* testSphere = new Sphere(1, Vector3(0,0,0));
	Box* testBox = new Box(Vector3(0, 0, 0), 1, 1, 1);
	Box* testBox2 = new Box(Vector3(0, -3, 0), 8, 0.1f, 8);
	testBox2->SetDiffuse(Color(1, 0, 0));

	m_SceneObjects.push_back(testSphere);
	m_SceneObjects.push_back(testBox);
	m_SceneObjects.push_back(testBox2);
	m_InitTime = clock();

	DirectionalLight* testLight = new DirectionalLight(Color(1, 1, 1), 0.2f, Vector3(1,-1,-1));
	PointLight* pointTest = new PointLight(Color(1, 0.0f, 1), 0.6f, Vector3(2, 3, -2), 10);

	m_SceneLights = std::vector<Light*>();
	m_SceneLights.push_back(testLight);
	m_SceneLights.push_back(pointTest);

	m_pCamera = _cam;
	m_pCamera->SetPosition(Vector3(0,2,10));
	m_pCamera->SetRotation(0, -0.3, 0);
}

void Scene::Update()
{
	clock_t time = clock();	
	double deltaTime = (double)(time - m_PrevTime)/CLOCKS_PER_SEC;
	double totalTime = (double)(time - m_InitTime)/CLOCKS_PER_SEC;

	m_SceneObjects[1]->SetPosition(Vector3(-5 * cosf(totalTime), 0, -5 * sinf(totalTime)));

	//m_pCamera->SetPosition(Vector3(0,0, sinf(totalTime) * 5 + 10));
	//m_pCamera->SetRotation(sinf(totalTime*2)/2 + 3.141f,0, 0);

	//m_pCamera->LookAt(Vector3(0, 0, 10), Vector3(0,0,0), Vector3(0,-1,0));

	m_PrevTime = time;
}

Color Scene::Intersect(const DirectX::SimpleMath::Ray& _ray)
{
	float minDist = FLT_MAX;
	Intersection minIntersect;
	Intersection intersect;
	bool intersectFound = false;

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
	else
	{
		Color retColor = Color(0,0,0);
		Color ambientColor = Color(0.05f, 0.05f, 0.05f);
		
		for(auto light = m_SceneLights.begin(); light != m_SceneLights.end(); light++)
		{
			Vector3 lightDir = (*light)->GetDirection(minIntersect.position);
			Ray shadowRay(minIntersect.position + lightDir * 0.0001f, lightDir);
			bool inShadow = false;
			for(auto obj = m_SceneObjects.begin(); obj != m_SceneObjects.end(); obj++)
			{
				if((*obj)->Intersect(shadowRay, intersect))
				{
					inShadow = true;
					break;
				}
			}
			
			float diffuseFactor = 0;
			if(!inShadow)
			{
				diffuseFactor = minIntersect.normal.Dot(lightDir);
				if(diffuseFactor < 0)
					diffuseFactor = 0;
			}

			float dist = (*light)->GetDistance(minIntersect.position);
			float range = (*light)->GetRange();
			if(dist > range)
				dist = range;
			float strength = (range - dist)/range;

			Color diffuse = diffuseFactor * strength * (*light)->GetColor();

			retColor += minIntersect.color * diffuse;
		}

		retColor += minIntersect.color * ambientColor;
		retColor.Saturate();
		return retColor;

	}
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
