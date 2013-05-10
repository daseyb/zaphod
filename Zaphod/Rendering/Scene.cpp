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
	//Initialize object lists
	m_SceneObjects = std::vector<BaseObject*>();
	Sphere* testSphere = new Sphere(1, Vector3(0,0,0));

	//Build a few test objects and materials
	Box* testBox = new Box(Vector3(0, 0, 0), 1, 1, 1);
	Box* testBox2 = new Box(Vector3(0, -1.05f, 0), 20, 0.1f, 20);
	
	Material whiteMat;
	whiteMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);
	whiteMat.SpecularColor = Color(1,1,1);
	whiteMat.ReflectionColor = Color(1,1,1);
	whiteMat.DiffuseFactor = 1;
	whiteMat.SpecularFactor = 1.0f;
	whiteMat.ReflectionFactor = 0.3f;

	testSphere->SetMaterial(whiteMat);
	testBox->SetMaterial(whiteMat);

	Material floorMat;
	floorMat.DiffuseColor = Color(1.0f, 0.5f, 0.5f);
	floorMat.SpecularColor = Color(1,1,1);
	floorMat.ReflectionColor = Color(1,1,1);
	floorMat.DiffuseFactor = 1;
	floorMat.SpecularFactor = 0.0f;
	floorMat.ReflectionFactor = 0.5f;

	testBox2->SetMaterial(floorMat);

	m_SceneObjects.push_back(testSphere);
	m_SceneObjects.push_back(testBox);
	m_SceneObjects.push_back(testBox2);
	
	//Build a few test lights
	PointLight* pointTest = new PointLight(Color(1, 1.0f, 0.7f), 0.9f, Vector3(-2, 3, 2), 15);
	PointLight* pointTest2 = new PointLight(Color(0.5f, 0.5f, 1.0f), 0.5f, Vector3(2, 2, -1), 30);

	m_SceneLights = std::vector<Light*>();
	m_SceneLights.push_back(pointTest2);
	m_SceneLights.push_back(pointTest);

	//Set the start time
	m_InitTime = clock();

	//Set the camera pointer and move the camera to it's start position
	m_pCamera = _cam;
	m_pCamera->SetPosition(Vector3(0,2,10));
	m_pCamera->SetRotation(0, -0.3, 0);
}

void Scene::Update()
{
	//Update the time values
	clock_t time = clock();	
	double deltaTime = (double)(time - m_PrevTime)/CLOCKS_PER_SEC;
	double totalTime = (double)(time - m_InitTime)/CLOCKS_PER_SEC;

	//Update objects
	m_SceneObjects[1]->SetPosition(Vector3(-5 * cosf(totalTime), 0, -5 * sinf(totalTime)));

	//m_pCamera->SetPosition(Vector3(0,0, sinf(totalTime) * 5 + 10));
	//m_pCamera->SetRotation(sinf(totalTime*2)/2 + 3.141f,0, 0);

	//m_pCamera->LookAt(Vector3(0, 0, 10), Vector3(0,0,0), Vector3(0,-1,0));

	m_PrevTime = time;
}

//Intersect a ray with the scene (currently no optimization)
Color Scene::Intersect(const DirectX::SimpleMath::Ray& _ray) const
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
	else
	{
		//Calculate lighting
		Color retColor = Color(0,0,0);
		Color ambientColor = Color(0.0f, 0.0f, 0.0f);
		
		//For each light
		for(auto light = m_SceneLights.begin(); light != m_SceneLights.end(); light++)
		{
			//Cast shadow ray
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

			//In shadow, no lighting to be done
			if(inShadow)
				continue;
			
			//Diffuse lighting
			float diffuseFactor = minIntersect.normal.Dot(lightDir);
			if(diffuseFactor < 0)
				diffuseFactor = 0;

			float dist = (*light)->GetDistance(minIntersect.position);
			float range = (*light)->GetRange();
			if(dist > range)
				dist = range;
			float strength = pow((range - dist)/range, 2);

			Color diffuse = diffuseFactor * strength * (*light)->GetColor();
			Color specular(0,0,0);

			//Specular lighting
			if(minIntersect.material.SpecularFactor > 0)
			{
				float specularFactor = pow(Vector3::Reflect(lightDir, minIntersect.normal).Dot(_ray.direction), 50);
				if(specularFactor < 0)
					specularFactor = 0;
				else if(specularFactor > 1)
					specularFactor = 1;

				specular = specularFactor * minIntersect.material.SpecularColor * minIntersect.material.SpecularFactor;
			}

			retColor += (minIntersect.material.DiffuseColor * diffuse + specular);
		}

		//Reflection calculation (not recursive yet)
		Color reflection(0,0,0);
		if(minIntersect.material.ReflectionFactor > 0)
		{
			reflection = Intersect(Ray(minIntersect.position, Vector3::Reflect(_ray.direction, minIntersect.normal)));
		}
		
		//Add ambient color;
		retColor += (minIntersect.material.DiffuseColor * ambientColor);

		//Weight color form lighting with color from reflections
		retColor = retColor * (1.0f - minIntersect.material.ReflectionFactor) + reflection * minIntersect.material.ReflectionFactor;
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
