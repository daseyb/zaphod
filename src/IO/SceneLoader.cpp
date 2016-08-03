#include "SceneLoader.h"
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <sstream>
#include <fstream>
#include <iostream>

#include "ObjLoader.h"

#include "../Geometry/Triangle.h"

#include "../Rendering/Materials/Material.h"
#include "../Rendering/Materials/DiffuseMaterial.h"
#include "../Rendering/Materials/SpecularMaterial.h"
#include "../Rendering/Materials/EmissionMaterial.h"

#include "../Objects/BaseObject.h"
#include "../Objects/Sphere.h"
#include "../Objects/Box.h"
#include "../Objects/Mesh.h"

#include "../Rendering/Cameras/Camera.h"
#include "../Rendering/Cameras/PinholeCamera.h"
#include "../Rendering/Cameras/PhysicallyBasedCamera.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

enum ParsedObjectType { Mat, Cam, Obj };

struct ObjectData {
  std::string MaterialName;
  BaseObject *Object;
};

struct ParsedObject {
  ParsedObjectType Type;
  std::string Name;
  void *Data;
};

struct pairhash {
public:
  template <typename T, typename U>
  std::size_t operator()(const std::pair<T, U> &x) const {
    return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
  }
};

// trim from start
static inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(),
          std::find_if(s.begin(), s.end(),
                       std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       std::not1(std::ptr_fun<int, int>(std::isspace)))
              .base(),
          s.end());
  return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) { return ltrim(rtrim(s)); }

std::unordered_map<std::string, std::string>
ParseDictionary(std::ifstream &file) {
  std::unordered_map<std::string, std::string> dict;

  std::string line;
  do {
    std::getline(file, line);
    line = trim(line);
    if (line.size() == 0)
      break;

    std::stringstream ss(line);
    std::string key;
    std::getline(ss, key, ':');
    std::string value;
    std::getline(ss, value, ':');
    key = trim(key);
    value = trim(value);

    assert(key.size() > 0 && value.size() > 0);

    dict.insert({key, value});
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

template <typename T>
T GetValue(const std::unordered_map<std::string, std::string> &values,
           std::string key) {
  return GetValue(values, key, T{});
}

template <typename T>
T GetValue(const std::unordered_map<std::string, std::string> &values,
           std::string key, T def) {
  if (values.find(key) == values.end())
    return def;
  return values.at(key);
}

template <>
float GetValue<float>(
    const std::unordered_map<std::string, std::string> &values, std::string key,
    float def) {
  if (values.find(key) == values.end())
    return def;
  return ParseFloat(values.at(key));
}

template <>
Color GetValue<Color>(
    const std::unordered_map<std::string, std::string> &values, std::string key,
    Color def) {
  if (values.find(key) == values.end())
    return def;
  return ParseColor(values.at(key));
}

template <>
Vector3
GetValue<Vector3>(const std::unordered_map<std::string, std::string> &values,
                  std::string key, Vector3 def) {
  if (values.find(key) == values.end())
    return def;
  return ParseVector(values.at(key));
}

bool LoadScene(const std::string &sceneFileName,
               std::vector<BaseObject *> &loadedObjects,
               Camera **loadedCamera) {
  std::string sceneFileFolder =
      sceneFileName.substr(0, sceneFileName.find_last_of("\\/"));

  std::ifstream sceneFile(sceneFileName, std::ifstream::in);

  std::unordered_map<std::pair<ParsedObjectType, std::string>, ParsedObject,
                     pairhash> parsedObjects;

  if (!sceneFile.is_open()) {
    return false;
  }

  auto ParseMaterial = [](
      const std::unordered_map<std::string, std::string> &values)
      -> Material * {
        auto type = GetValue<std::string>(values, "type");
        auto color = GetValue<Color>(values, "color");
        if (type == "emission") {
          return new EmissionMaterial(
              color * GetValue<float>(values, "strength", 1.0f));
        } else if (type == "diffuse") {
          return new DiffuseMaterial(color);
        } else if (type == "specular") {
          return new SpecularMaterial(color, GetValue<float>(values, "kd", 0.5f),
                                      GetValue<float>(values, "ks", 0.5f),
                                      GetValue<float>(values, "kt", 0.0f),
                                      GetValue<float>(values, "roughness", 0.0f));
        } else {
          std::cout << "Unknown material type: " << type << std::endl;
          return nullptr;
        }
      };

  auto ParseSceneObject = [sceneFileFolder](
      const std::unordered_map<std::string, std::string>
          &values) -> ObjectData * {
    BaseObject *result;
    auto type = GetValue<std::string>(values, "type");
    auto pos = GetValue<Vector3>(values, "position");
    if (type == "box") {
      result = new Box(pos, GetValue<Vector3>(values, "extends"));
    } else if (type == "sphere") {
      result = new Sphere(pos, GetValue<float>(values, "radius"));
    } else if (type == "mesh") {
      auto meshFile =
          sceneFileFolder + "\\" + GetValue<std::string>(values, "file");
      std::vector<Triangle> tris;
      std::vector<Vector3> verts;
      std::vector<Vector3> normals;
      std::vector<Vector2> uvs;
      bool smooth;

      if (!LoadObj(meshFile, tris, verts, normals, uvs, smooth)) {
        std::cout << "Could not load object at " << meshFile << std::endl;
        return nullptr;
      }

      result = new Mesh(pos, tris, verts, normals, uvs, smooth);
    } else {
      std::cout << "Unknown object type: " << type << std::endl;
      return nullptr;
    }

    result->SetRotation(GetValue<Vector3>(values, "rotation"));
    result->SetScale(GetValue<Vector3>(values, "scale", Vector3(1, 1, 1)));

    return new ObjectData{GetValue<std::string>(values, "material"), result};
  };

  auto ParseCamera = [](
      const std::unordered_map<std::string, std::string> &values,
      Camera **cam) {
    auto camType = GetValue<std::string>(values, "type");

    if (camType == "pinhole") {
      *cam = new PinholeCamera(GetValue<float>(values, "fov"));
    } else if (camType == "physically_based") {
      auto lensRadius = GetValue<float>(values, "lense_radius");
      auto dofDistance = GetValue<float>(values, "dof_distance");
      auto fov = GetValue<float>(values, "fov");

      if (lensRadius == 0 || dofDistance == 0 || fov == 0) {
        std::cout << "Invalid camera parameters!" << std::endl;
        return false;
      }

      *cam = new PhysicallyBasedCamera(dofDistance, lensRadius, fov);

    } else {
      std::cout << "Unknown camera type: " << camType << std::endl;
      return false;
    }

    (*cam)->SetPosition(GetValue<Vector3>(values, "position"));
    auto rotation = GetValue<Vector3>(values, "rotation");
    (*cam)->SetRotation(rotation);
    return true;
  };

  while (!sceneFile.eof()) {
    std::string line;
    std::getline(sceneFile, line);
    line = trim(line);

    if (line.size() == 0 || line[0] == '#') {
      continue;
    }

    std::stringstream lineStream(line);

    std::string definitionType, name;
    lineStream >> definitionType >> name;

    ParsedObject obj;
    obj.Name = name;

    auto dict = ParseDictionary(sceneFile);

    if (definitionType == "Material") {
      obj.Data = ParseMaterial(dict);
      obj.Type = ParsedObjectType::Mat;
    } else if (definitionType == "Object") {
      obj.Data = ParseSceneObject(dict);
      obj.Type = ParsedObjectType::Obj;
    } else if (definitionType == "Camera") {
      if (!ParseCamera(dict, loadedCamera)) {
        return false;
      }
      obj.Type = ParsedObjectType::Cam;
      obj.Data = nullptr;
    }

    parsedObjects.insert({{obj.Type, name}, obj});
  }

  DiffuseMaterial defaultDiffuse(Color(1.0f, 1.0f, 1.0f));
  for (auto &obj : parsedObjects) {
    if (!obj.second.Data) {
      continue;
    }

    auto objType = obj.first.first;
    auto objName = obj.first.second;

    switch (objType) {
    case ParsedObjectType::Obj:
      ObjectData data = *(ObjectData *)obj.second.Data;
      Material *materialToUse = &defaultDiffuse;
      if (data.MaterialName == "") {
        std::cout << "Object " << objName
                  << " has no material assigned. Using default diffuse!"
                  << std::endl;
      } else if (parsedObjects.find({Mat, data.MaterialName}) ==
                 parsedObjects.end()) {
        std::cout << "Could not find the matrial " << data.MaterialName
                  << " assigned to object " << objName
                  << ". Using default diffuse!" << std::endl;
      } else {
        auto &mat = parsedObjects[{Mat, data.MaterialName}];
        if (mat.Type != ParsedObjectType::Mat) {
          std::cout << "The matrial " << data.MaterialName
                    << " assigned to object " << objName
                    << " is not valid! Using default diffuse!" << std::endl;
        } else {
          materialToUse = (Material *)mat.Data;
        }
      }

      data.Object->SetMaterial(materialToUse);
      loadedObjects.push_back(data.Object);
    }
  }

  return true;
}