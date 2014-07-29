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
	Mesh* monkey = new Mesh(Vector3(-2, 0.25f, 6), "Data/test_smooth.obj");

	Material whiteMat;
	whiteMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);
	whiteMat.SpecularColor = Color(1,1,1);
	whiteMat.ReflectionColor = Color(1,1,1);
	whiteMat.DiffuseFactor = 1;
	whiteMat.SpecularFactor = 0.0f;
	whiteMat.ReflectionFactor = 0.0f;
	whiteMat.Transparency = 0.0f;
	whiteMat.IOR = 1.0f;

	Material transparentMat;
	transparentMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);
	transparentMat.SpecularColor = Color(1,1,1);
	transparentMat.ReflectionColor = Color(0,1,0);
	transparentMat.DiffuseFactor = 1.0f;
	transparentMat.SpecularFactor = 1.0f;
	transparentMat.ReflectionFactor = 0.2f;
	transparentMat.Transparency = 1.0f;
	transparentMat.IOR = 1.1f;

	testBox->SetMaterial(whiteMat);

	Material floorMat;
	floorMat.DiffuseColor = Color(1.0f, 1.0f, 1.0f);
	floorMat.SpecularColor = Color(1,1,1);
	floorMat.ReflectionColor = Color(1,1,1);
	floorMat.DiffuseFactor = 1;
	floorMat.SpecularFactor = 0.0f;
	floorMat.ReflectionFactor = 0.0f;
	floorMat.Transparency = 0.0f;
	floorMat.IOR = 1.0f;

	Material chromeMatBase;
	chromeMatBase.DiffuseColor = Color(0.5f, 0.5f, 0.5f);
	chromeMatBase.SpecularColor = Color(1, 1, 1);
	chromeMatBase.ReflectionColor = Color(1, 1, 1);
	chromeMatBase.DiffuseFactor = 0.4f;
	chromeMatBase.SpecularFactor = 0.0f;
	chromeMatBase.ReflectionFactor = 0.6f;
	chromeMatBase.Transparency = 0.0f;
	chromeMatBase.IOR = 1.0f;
	chromeMatBase.Glossyness = 0.8f;

	Material chromeMatRed = chromeMatBase;
	chromeMatRed.DiffuseColor = Color(0.5f, 0.0f, 0.0f);

	Material chromeMatBlue = chromeMatBase;
	chromeMatBlue.DiffuseColor = Color(0.0f, 0.0f, 0.5f);

	testBox2->SetMaterial(floorMat);
	testBox3->SetMaterial(floorMat);
	testBox4->SetMaterial(floorMat);
	testSphere->SetMaterial(transparentMat);
	monkey->SetMaterial(whiteMat);
	testSphere2->SetMaterial(chromeMatBlue);
	testSphere3->SetMaterial(chromeMatRed);

	m_SceneObjects.push_back(monkey);
	m_SceneObjects.push_back(testBox);
	m_SceneObjects.push_back(testSphere);
	m_SceneObjects.push_back(testBox2);
	m_SceneObjects.push_back(testBox3);
	m_SceneObjects.push_back(testBox4);
	m_SceneObjects.push_back(testBox5);
	m_SceneObjects.push_back(testSphere2);
	m_SceneObjects.push_back(testSphere3);
	
	//Build a few test lights
	PointLight* pointTest = new PointLight(Color(1.0, 0.5f, 0.0f), 1.9f, Vector3(0, 2, -6), 20);
	PointLight* pointTest2 = new PointLight(Color(0.2f, 1.0f, 0.2f), 0.9f, Vector3(-2, 2, 2), 9);
	PointLight* pointTest3 = new PointLight(Color(0.0f, 0.66f, 1.0f), 1.5f, Vector3(0, 2, 10), 20);

	DirectionalLight* dirTest = new DirectionalLight(Color(1, 1, 1), 0.2f, Vector3(-1, -1, -2));

	m_SceneLights = std::vector<Light*>();
	m_SceneLights.push_back(pointTest2);
	m_SceneLights.push_back(pointTest);
	m_SceneLights.push_back(pointTest3);
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

	//Update objects
	//m_SceneObjects[1]->SetPosition(Vector3(-5 * cosf(totalTime), 0.25f, -5 * sinf(totalTime)));

	//m_pCamera->SetPosition(Vector3(0,0, sinf(totalTime) * 5 + 10));
	//m_pCamera->SetRotation(sinf(totalTime*2)/2 + 3.141f,0, 0);

	//m_pCamera->LookAt(Vector3(0, 0, 10), Vector3(0,0,0), Vector3(0,-1,0));

	m_PrevTime = time;
}

float GetRnd()
{
	return (float)rand()/RAND_MAX;
}

Vector3 RandomOffset(Vector3 _dir, float _factor) {
	return _dir + Vector3((GetRnd() - 0.5f) * 0.2f * _factor, (GetRnd() - 0.5f) * 0.2f * _factor, (GetRnd() - 0.5f) * 0.2f * _factor);
}

//Intersect a ray with the scene (currently no optimization)
Color Scene::Intersect(const DirectX::SimpleMath::Ray& _ray, int _depth) const
{
	if(_depth < 0)
		return Color(0,0,0);

	const int SHADOW_SAMPLES = 5;
	const int GLOSSY_SAMPLES = 2;

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
		for(auto light : m_SceneLights)
		{
			Color shadowColor = Color(1,1,1);
			float shadowTransparency = 1;
			bool inShadow = false;
			Vector3 lightDir = light->GetDirection(minIntersect.position);
			float lightDist = light->GetDistance(minIntersect.position);
			Intersection shadowIntersect;

			int inShadowCount = 0;
			for(int i = 0; i < SHADOW_SAMPLES; i++) {
				//Cast shadow ray
				Vector3 rayDir = RandomOffset(lightDir, 0.5f);
				rayDir.Normalize();
				Ray shadowRay(minIntersect.position + rayDir * 0.0001f, rayDir);

				for(auto obj = m_SceneObjects.begin(); obj != m_SceneObjects.end(); obj++)
				{
					if ((*obj)->Intersect(shadowRay, shadowIntersect) && (shadowIntersect.position - minIntersect.position).Length() < lightDist)
					{
						if (shadowIntersect.material.Transparency == 0)
						{
							inShadow = true;
							break;
						}
						else
						{
							shadowColor *= shadowIntersect.material.DiffuseColor;
							shadowTransparency *= shadowIntersect.material.Transparency;
						}
					}
				}

				if(inShadow)
					inShadowCount++;
			}

			//In shadow, no lighting to be done
			if(inShadowCount == SHADOW_SAMPLES)
				continue;

			//ambientColor = ambientColor * (shadowColor*shadowTransparency);
			
			//Diffuse lighting
			float diffuseFactor = minIntersect.normal.Dot(lightDir);
			if(diffuseFactor < 0)
				diffuseFactor = 0;

			float range = light->GetRange();
			if (lightDist > range)
				lightDist = range;
			float strength = pow((range - lightDist) / range, 2);

			Color diffuse = diffuseFactor * strength * light->GetColor();
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

			retColor += (minIntersect.material.DiffuseColor * diffuse + specular) * (shadowTransparency * shadowColor) * (1.0f - (float)inShadowCount/SHADOW_SAMPLES);
		}

		//Angles for fresenel equations
		float ot, oi;

		//Reflection calculation
		Color reflection(0,0,0);
		if(minIntersect.material.ReflectionFactor > 0)
		{
			Vector3 reflectionDir = Vector3::Reflect(_ray.direction, minIntersect.normal);
			reflection = Intersect(Ray(minIntersect.position + reflectionDir * 0.001f, reflectionDir) , _depth - 1);
			oi = acos(reflectionDir.Dot(-_ray.direction))/2;
			if(minIntersect.material.Glossyness > 0) {
				for (int i = 0; i < GLOSSY_SAMPLES; i++) {
					reflectionDir = RandomOffset(Vector3::Reflect(_ray.direction, minIntersect.normal), minIntersect.material.Glossyness);
					reflectionDir.Normalize();
					reflection += Intersect(Ray(minIntersect.position + reflectionDir * 0.001f, reflectionDir) , _depth - 1) * minIntersect.material.Glossyness;
				}

				reflection *= 1.0f / GLOSSY_SAMPLES;
			}
		}

		
		Color refraction(0,0,0);
		bool totalReflection = true;
		if(minIntersect.material.Transparency > 0)
		{
			Vector3 refractNormal = minIntersect.normal;
			if(_ray.direction.Dot(minIntersect.normal) > 0)
				refractNormal *= -1;

			Vector3 transmissionDir = Vector3::Refract(_ray.direction, refractNormal, 1.0f/minIntersect.material.IOR);
			transmissionDir.Normalize();
			Ray transmissionRay = Ray(minIntersect.position + transmissionDir * 0.001f, transmissionDir);
			if(transmissionRay.direction.Length() > 0)
			{
				refraction = Intersect(transmissionRay, _depth - 1);
				ot = acos(refractNormal.Dot(-transmissionDir));
				totalReflection = false;
			}
		}

		//Fresnel Equations
		float r;
		if(totalReflection)
		{
			r = 0;
		}
		else
		{
			float rs, rp;
			rs = sin(ot-oi)/sin(ot+oi);
			rs *= rs;

			rp = tan(ot-oi)/tan(ot+oi);
			rp *= rp;
			r = 1.0f - (rs + rp)/2;
		}

		/*reflection *= (1.0f - r);
		refraction *= r;*/

		//Add ambient color;
		retColor += (minIntersect.material.DiffuseColor * ambientColor);

		//Weight color form lighting with color from reflections
		retColor = retColor * (1.0f - minIntersect.material.ReflectionFactor) + reflection * minIntersect.material.ReflectionFactor;
		retColor = retColor * (1.0f - minIntersect.material.Transparency * r)  + refraction * minIntersect.material.Transparency * r;
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
