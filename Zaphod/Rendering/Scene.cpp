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
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include "Materials/Material.h"
#include "Materials/DiffuseMaterial.h"
#include "Materials/SpecularMaterial.h"
#include "Materials/EmissionMaterial.h"
#include <unordered_map>
#include <algorithm>
#include <functional> 
#include <cctype>
#include <locale>

#define USE_LIGHTCACHE 0

using namespace DirectX;
using namespace DirectX::SimpleMath;


enum ParsedObjectType {
	Mat,
	Cam,
	Obj
};

struct ObjectData {
	std::string MaterialName;
	BaseObject* Object;
};

struct ParsedObject {
	ParsedObjectType Type;
	std::string Name;
	void* Data;
};

struct pairhash {
public:
	template <typename T, typename U>
	std::size_t operator()(const std::pair<T, U> &x) const
	{
		return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
	}
};


// trim from start
static inline std::string &ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
	return ltrim(rtrim(s));
}

std::unordered_map<std::string, std::string> ParseDictionary(std::ifstream& file) {
	std::unordered_map<std::string, std::string> dict;

	std::string line;
	do {
		std::getline(file, line);
		line = trim(line);
		if (line.size() == 0) break;

		std::stringstream ss(line);
		std::string key;
		std::getline(ss, key, ':');
		std::string value;
		std::getline(ss, value, ':');
		key = trim(key);
		value = trim(value);

		assert(key.size() > 0 && value.size() > 0);

		dict.insert({ key, value });
	} while (!file.eof());

	return dict;
}

Color ParseColor(std::string value) {
	std::stringstream valueStream(value);
	float r, g, b;
	valueStream >> r >> g >> b;
	return Color(r, g, b);
}

Vector3 ParseVector(std::string value) {
	std::stringstream valueStream(value);
	float x, y, z;
	valueStream >> x >> y >> z;
	return Vector3(x, y, z);
}

float ParseFloat(std::string value) {
	std::stringstream valueStream(value);
	float val;
	valueStream >> val;
	return val;
}


template<typename T>
T GetValue(const std::unordered_map<std::string, std::string>& values, std::string key) {
	return GetValue(values, key, T{});
}

template<typename T>
T GetValue(const std::unordered_map<std::string, std::string>& values, std::string key, T def) {
	if (values.find(key) == values.end()) return def;
	return values.at(key);
}

template<>
float GetValue<float>(const std::unordered_map<std::string, std::string>& values, std::string key, float def) {
	if (values.find(key) == values.end()) return def;
	return ParseFloat(values.at(key));
}


template<>
Color GetValue<Color>(const std::unordered_map<std::string, std::string>& values, std::string key, Color def) {
	if (values.find(key) == values.end()) return def;
	return ParseColor(values.at(key));
}

template<>
Vector3 GetValue<Vector3>(const std::unordered_map<std::string, std::string>& values, std::string key, Vector3 def) {
	if (values.find(key) == values.end()) return def;
	return ParseVector(values.at(key));
}

Scene::Scene(Camera* _cam, const char* _sceneFile)
{
	//Initialize object lists
	m_SceneObjects = std::vector<BaseObject*>();

	std::string sceneFileStr(_sceneFile);
	std::string sceneFileFolder = sceneFileStr.substr(0, sceneFileStr.find_last_of("\\/"));

	std::ifstream sceneFile(sceneFileStr, std::ifstream::in);

	std::unordered_map<std::pair<ParsedObjectType, std::string>, ParsedObject, pairhash> parsedObjects;


	if (!sceneFile.is_open()) {
		throw "Could not open the scene file!";
	}

	auto ParseMaterial = [this](const std::unordered_map<std::string, std::string>& values) -> Material* {
		auto type = GetValue<std::string>(values, "type");
		auto color = GetValue<Color>(values, "color");
		if (type == "emission") {
			return new EmissionMaterial(color * GetValue<float>(values, "strength", 1.0f));
		}
		else if (type == "diffuse") {
			return new DiffuseMaterial(color);
		}
		else if (type == "specular") {
			return new SpecularMaterial(color, GetValue<float>(values, "kd"), GetValue<float>(values, "ks"), GetValue<float>(values, "roughness"));
		}

		return nullptr;
	};

	auto ParseSceneObject = [sceneFileFolder](const std::unordered_map<std::string, std::string>& values) -> ObjectData* {
		BaseObject* result;
		auto type = GetValue<std::string>(values, "type");
		auto pos = GetValue<Vector3>(values, "position");
		if (type == "box") {
			result = new Box(pos, GetValue<Vector3>(values, "extends"));
		}
		else if (type == "sphere") {
			result = new Sphere(pos, GetValue<float>(values, "radius"));
		}
		else if (type == "mesh") {
			result = new Mesh(pos, sceneFileFolder + "\\" + GetValue<std::string>(values, "file"));
		}

		result->SetRotation(GetValue<Vector3>(values, "rotation"));
		result->SetScale(GetValue<Vector3>(values, "scale", Vector3(1, 1, 1)));

		return new ObjectData{GetValue<std::string>(values, "material"), result };
	};

	auto ParseCamera = [this](const std::unordered_map<std::string, std::string>& values, Camera* cam) {
		cam->SetPosition(GetValue<Vector3>(values, "position"));
		auto rotation = GetValue<Vector3>(values, "rotation");
		cam->SetRotation(rotation);
	};
	
	while (!sceneFile.eof()) {
		std::string line;
		std::getline(sceneFile, line);
		line = trim(line);
		if (line.size() == 0 || line[0] == '#') continue;

		std::stringstream lineStream(line);

		std::string definitionType, name;
		lineStream >> definitionType >> name;

		ParsedObject obj;
		obj.Name = name;

		auto dict = ParseDictionary(sceneFile);

		if (definitionType == "Material") {
			obj.Data = ParseMaterial(dict);
			obj.Type = ParsedObjectType::Mat;
		}
		else if (definitionType == "Object") {
			obj.Data = ParseSceneObject(dict);
			obj.Type = ParsedObjectType::Obj;
		}
		else if (definitionType == "Camera") {
			ParseCamera(dict, _cam);
			obj.Type = ParsedObjectType::Cam;
			obj.Data = nullptr;
		}

		parsedObjects.insert({ {obj.Type, name}, obj });
	}

	DiffuseMaterial defaultDiffuse(Color(1.0f, 1.0f, 1.0f));
	for (auto& obj : parsedObjects) {
		if (!obj.second.Data) {
			continue;
		}

		auto objType = obj.first.first;
		auto objName = obj.first.second;

		switch (objType)
		{
		case ParsedObjectType::Obj:
			ObjectData data = *(ObjectData*)obj.second.Data;
			Material* materialToUse = &defaultDiffuse;
			if (data.MaterialName == "") {
				std::cout << "Object " << objName << " has no material assigned. Using default diffuse!" << std::endl;
			}
			else if (parsedObjects.find({ Mat, data.MaterialName }) == parsedObjects.end()) {
				std::cout << "Could not find the matrial " << data.MaterialName << " assigned to object " << objName << ". Using default diffuse!" << std::endl;
			}
			else {
				auto& mat = parsedObjects[{Mat, data.MaterialName}];
				if (mat.Type != ParsedObjectType::Mat) {
					std::cout << "The matrial " << data.MaterialName << " assigned to object " << objName << " is not valid! Using default diffuse!" << std::endl;
				}
				else {
					materialToUse = (Material*)mat.Data;
				}
			}

			data.Object->SetMaterial(materialToUse);
			m_SceneObjects.push_back(data.Object);
		}
	}

	m_SceneLights = std::vector<BaseObject*>();

	m_TotalLightWeight = 0;
	for (auto obj : m_SceneObjects) {
		if (obj->GetMaterial()->IsLight()) {
			m_SceneLights.push_back(obj);
			float weight = obj->CalculateWeight();
			if (weight == 0) {
				continue;
			}
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
