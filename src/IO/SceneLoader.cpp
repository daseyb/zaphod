#include "SceneLoader.h"
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <sstream>
#include <fstream>
#include <iostream>

#include <pugixml.hpp>

#include "ObjLoader.h"
#include "AbcLoader.h"

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
ParseDictionary(std::istream &file) {
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

bool hasEnding(std::string const &fullString, std::string const &ending) {
  if (fullString.length() >= ending.length()) {
    return (0 ==
            fullString.compare(fullString.length() - ending.length(),
                               ending.length(), ending));
  } else {
    return false;
  }
}

bool LoadZSF(std::istream& sceneStream, std::string sceneFileName, std::vector<BaseObject *> &loadedObjects, Camera **loadedCamera) {

    std::string sceneFileFolder =
        sceneFileName.substr(0, sceneFileName.find_last_of("\\/"));

    std::unordered_map<std::pair<ParsedObjectType, std::string>, ParsedObject,
        pairhash> parsedObjects;


    auto ParseMaterial = [](const std::unordered_map<std::string, std::string>
        &values) -> Material * {
        auto type = GetValue<std::string>(values, "type");
        auto color = GetValue<Color>(values, "color");
        if (type == "emission") {
            return new EmissionMaterial(color *
                GetValue<float>(values, "strength", 1.0f));
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
        &values) -> std::vector<ObjectData*> {
        BaseObject *root = nullptr;
        std::vector<ObjectData*> result;
        auto type = GetValue<std::string>(values, "type");
        auto pos = GetValue<Vector3>(values, "position");
        if (type == "box") {
            root = new Box(pos, GetValue<Vector3>(values, "extends"));
            result.push_back(new ObjectData{ "", root });
        } else if (type == "sphere") {
            root = new Sphere(pos, GetValue<float>(values, "radius"));
            result.push_back(new ObjectData{ "",  root });
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
                return{};
            }

            root = new Mesh(pos, tris, verts, normals, uvs, smooth);
            result.push_back(new ObjectData{ "", root });
        } else if (type == "alembic") {
            auto abcFile =
                sceneFileFolder + "\\" + GetValue<std::string>(values, "file");

            if (!LoadAbc(abcFile, &root, result)) {
                std::cout << "Could not load object at " << abcFile << std::endl;
                return{};
            }
        } else {
            std::cout << "Unknown object type: " << type << std::endl;
            return{};
        }

        root->SetPosition(GetValue<Vector3>(values, "position"));
        root->SetRotation(GetValue<Vector3>(values, "rotation"));
        root->SetScale(GetValue<Vector3>(values, "scale", Vector3(1, 1, 1)));

        for (auto& res : result) {
            if (res->MaterialName == "") {
                res->MaterialName = GetValue<std::string>(values, "material");
            }
        }

        return result;
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

    while (!sceneStream.eof()) {
        std::string line;
        std::getline(sceneStream, line);
        line = trim(line);

        if (line.size() == 0 || line[0] == '#') {
            continue;
        }

        std::stringstream lineStream(line);

        std::string definitionType, name;
        lineStream >> definitionType >> name;

        ParsedObject obj;
        obj.Name = name;

        auto dict = ParseDictionary(sceneStream);

        if (definitionType == "Material") {
            obj.Data = ParseMaterial(dict);
            obj.Type = ParsedObjectType::Mat;
            parsedObjects.insert({ { obj.Type, name }, obj });
        } else if (definitionType == "Object") {
            auto sceneObjs = ParseSceneObject(dict);

            int i = 0;
            for (auto& sceneObj : sceneObjs) {
                obj.Data = sceneObj;
                obj.Type = ParsedObjectType::Obj;
                parsedObjects.insert({ { obj.Type, name + std::to_string(i++) }, obj });
            }

        } else if (definitionType == "Camera") {
            if (!ParseCamera(dict, loadedCamera)) {
                return false;
            }
            obj.Type = ParsedObjectType::Cam;
            obj.Data = nullptr;
            parsedObjects.insert({ { obj.Type, name }, obj });
        }

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

                RenderObject *renderObj = dynamic_cast<RenderObject *>(data.Object);
                if (renderObj) {
                    Material *materialToUse = &defaultDiffuse;
                    if (data.MaterialName == "") {
                        std::cout << "Object " << objName
                            << " has no material assigned. Using default diffuse!"
                            << std::endl;
                    } else if (parsedObjects.find({ Mat, data.MaterialName }) ==
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
                    renderObj->SetMaterial(materialToUse);
                }

                loadedObjects.push_back(data.Object);
        }
    }

    return true;
}


struct MitsubaIntegrator {
    enum class Type {
        VolPath,
    };
    Type type;
    uint32_t maxDepth;
    bool strictNormals;
};

struct MitsubaSampler {
    enum class Type {
        Sobol,
    };

    Type type;
    uint32_t sampleCount;
};

struct MitsubaRFilter {
    enum class Type {
        Tent
    };
    Type type;
};

struct MitsubaFilm {
    enum class Type {
        LDR,
    };

    Type type;

    uint32_t width;
    uint32_t height;

    std::string fileFormat;
    std::string pixelFormat;
    float gamma;
    bool banner;

    MitsubaRFilter rFilter;
};

struct MitsubaSensor {
    enum class Type {
        Perspective,
        Orthographic
    };

    Matrix transform;
    MitsubaSampler sampler;
    MitsubaFilm film;
};

struct MitsubaObject {
    std::string id;
};

struct MitsubaScene {
    std::string version;
    MitsubaIntegrator integrator;
    MitsubaSensor sensor;
    std::vector<std::unique_ptr<MitsubaObject>> objects;
};


struct MitsubaBsdf : MitsubaObject {
    enum class Type {
        Dielectric,
        Twosided,
    };

    Type type;
};

struct MitsubaBsdfDielectric : MitsubaBsdf {
    float intIOR;
    float extIOR;
};

struct MitsubaBsdfTwoSided : MitsubaBsdf {
    std::unique_ptr<MitsubaBsdf> front;
    std::unique_ptr<MitsubaBsdf> back;
};

struct MitsubaBsdfDiffuse {
    Color reflectance;
};

struct MitsubaShape {
    enum class Type {
        Obj,
        Disk,
        Rectangle
    };

    Type type;
    Matrix transform;
};

bool ParseMitsuba(std::string sceneFileName, MitsubaScene& scene) {
    pugi::xml_document doc;
    pugi::xml_parse_result xmlParse = doc.load_file(sceneFileName.c_str());

    if (xmlParse.status != pugi::status_ok) {
        std::cerr << "Couldn't parse Mitsuba scene file. " << xmlParse.description() << "\n";
        return false;
    }

    auto sceneNode = doc.child("scene");

    if (!sceneNode) {
        std::cerr << "No scene node in Mitsuba scene file.\n";
        return false;
    }

    scene.version = sceneNode.attribute("version").as_string();

    {
        auto integratorNode = sceneNode.child("integrator");
        if (!integratorNode) {
            std::cerr << "No integrator node in Mitsuba scene file.\n";
            return false;
        }

        MitsubaIntegrator integrator;
        std::string typeString = integratorNode.attribute("type").as_string();
        if (typeString == "volpath") {
            integrator.type = MitsubaIntegrator::Type::VolPath;
        }

        integrator.maxDepth = integratorNode.child("maxDepth").attribute("value").as_int();
        integrator.strictNormals = integratorNode.child("strictNormals").attribute("value").as_bool();
    }

    return true;
}


bool LoadMitsuba(std::istream& sceneStream, std::string sceneFileName, std::vector<BaseObject *> &loadedObjects, Camera **loadedCamera) {
    MitsubaScene scene;
    if (!ParseMitsuba(sceneFileName, scene)) return false;

    std::cerr << "Mitsuba import not yet supported, WIP\n";
    return false;
}

bool LoadScene(const std::string &sceneFileName,
               std::vector<BaseObject *> &loadedObjects,
               Camera **loadedCamera) {


  std::string sceneFileFormat = sceneFileName.substr(sceneFileName.find_last_of('.')+1);

  std::ifstream sceneFile(sceneFileName, std::ifstream::in);


  if (!sceneFile.is_open()) {
    return false;
  }

  if (sceneFileFormat == "zsf") {
      return LoadZSF(sceneFile, sceneFileName, loadedObjects, loadedCamera);
  } else if (sceneFileFormat == "xml") {
      // Assume Mitsuba
      return LoadMitsuba(sceneFile, sceneFileName, loadedObjects, loadedCamera);
  }

  std::cerr << "Unrecognized scene file format: " << sceneFileFormat << std::endl;
  return false;
}