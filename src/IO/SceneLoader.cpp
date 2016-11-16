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

Color ParseColor(std::string value, char sep = 0) {
  std::stringstream valueStream(value);
  float r, g, b;
  if (sep) {
      while (valueStream.peek() == sep) valueStream.get();
      valueStream >> r;
      while (valueStream.peek() == sep) valueStream.get();
      valueStream >> g;
      while (valueStream.peek() == sep) valueStream.get();
      valueStream >> b;
  } else {
      valueStream >> r >> g >> b;
  }
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

Matrix ParseMatrix(std::string value) {
    std::stringstream valueStream(value);
    Matrix val;
    valueStream 
        >> val._11 >> val._12 >> val._13 >> val._14
        >> val._21 >> val._22 >> val._23 >> val._24
        >> val._31 >> val._32 >> val._33 >> val._34
        >> val._41 >> val._42 >> val._43 >> val._44;
    return val.Transpose();
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
};

struct MitsubaIntegratorVolPath : MitsubaIntegrator {
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

struct MitsubaSamplerSobol : MitsubaSampler {
};

struct MitsubaRFilter {
    enum class Type {
        Tent
    };
    Type type;
};

struct MitsubaRFilterTent : MitsubaRFilter {
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

    std::unique_ptr<MitsubaRFilter> rFilter;
};


struct MitsubaFilmLDR : MitsubaFilm {
};

struct MitsubaSensor {
    enum class Type {
        Perspective,
        Orthographic
    };

    Type type;
    Matrix transform;
    std::unique_ptr<MitsubaSampler> sampler;
    std::unique_ptr<MitsubaFilm> film;
};

struct MitsubaSensorPerspective : MitsubaSensor {
    float fov;
};

struct MitsubaObject {
    enum class Type {
        BSDF,
        Shape
    };
    Type objType;
    std::string id;
};

struct MitsubaScene {
    std::string version;
    std::unique_ptr<MitsubaIntegrator> integrator;
    std::unique_ptr<MitsubaSensor> sensor;
    std::unordered_map<std::string, std::shared_ptr<MitsubaObject>> objects;
};

struct MitsubaColorSource {
    enum class Type {
        RGB,
        Texture
    };
    Type type;
};

struct MitsubaColorSourceRGB : MitsubaColorSource {
    Color color;
};

struct MitsubaColorSourceTexture : MitsubaColorSource {
    std::string filename;
    std::string filterType;
};

struct MitsubaBsdf : MitsubaObject {
    enum class Type {
        Dielectric,
        Twosided,
        Diffuse,
        RoughPlastic,
        RoughConductor,
        Conductor,
        Bumpmap
    };

    Type type;
};

struct MitsubaBsdfDielectric : MitsubaBsdf {
    float intIOR;
    float extIOR;
};

struct MitsubaBsdfTwoSided : MitsubaBsdf {
    std::shared_ptr<MitsubaBsdf> front;
    std::shared_ptr<MitsubaBsdf> back;
};

struct MitsubaBsdfDiffuse : MitsubaBsdf {
    std::unique_ptr<MitsubaColorSource> reflectance;
};

struct MitsubaBsdfRoughPlastic : MitsubaBsdf {
    float alpha;
    std::string distribution;
    float intIOR;
    float extIOR;
    bool nonLinear;
    std::unique_ptr<MitsubaColorSource> diffuseReflectance;
};

struct MitsubaBsdfRoughConductor : MitsubaBsdf {
    float alpha;
    std::string distribution;
    float extETA;
    float extIOR;
    std::unique_ptr<MitsubaColorSource> specularReflectance;
    std::unique_ptr<MitsubaColorSource> eta;
    std::unique_ptr<MitsubaColorSource> k;
};

struct MitsubaBsdfConductor : MitsubaBsdf {
    float extETA;
    std::unique_ptr<MitsubaColorSource> specularReflectance;
    std::unique_ptr<MitsubaColorSource> eta;
    std::unique_ptr<MitsubaColorSource> k;
};

struct MitsubaBsdfBumpmap : MitsubaBsdf {
    std::unique_ptr<MitsubaColorSource> map;
    std::shared_ptr<MitsubaBsdf> bsdf;
};

struct MitsubaEmitter {
    enum class Type {
        Area
    };

    std::unique_ptr<MitsubaColorSource> radiance;
};

struct MitsubaEmitterArea : MitsubaEmitter {
};

struct MitsubaShape : MitsubaObject {
    enum class Type {
        Obj,
        Disk,
        Rectangle,
        Cube,
        Sphere
    };

    Type type;
    Matrix transform;
    std::shared_ptr<MitsubaBsdf> material;
    std::shared_ptr<MitsubaEmitter> emitter;
};

struct MitsubaShapeObj : MitsubaShape {
    std::string filename;
    bool faceNormals;
};

struct MitsubaShapeDisk : MitsubaShape {
};

struct MitsubaShapeRectangle : MitsubaShape {
};

struct MitsubaShapeCube : MitsubaShape {
};

struct MitsubaShapeSphere : MitsubaShape {
    float radius;
    Vector3 center;
};

pugi::xml_attribute get_member(pugi::xml_node node, const char* name) {
    return node.find_child_by_attribute("name", name).attribute("value");
}

std::unique_ptr<MitsubaColorSource> ParseColorSource(pugi::xml_node node) {
    std::unique_ptr<MitsubaColorSource> result = nullptr;
    if (strcmp(node.name(), "rgb") == 0) {
        auto source = std::make_unique<MitsubaColorSourceRGB>();
        source->type = MitsubaColorSource::Type::RGB;
        source->color = ParseColor(node.attribute("value").as_string(), ',');
        result = std::move(source);
    } else if (strcmp(node.name(), "texture") == 0) {
        auto source = std::make_unique<MitsubaColorSourceTexture>();
        source->type = MitsubaColorSource::Type::Texture;
        source->filename = get_member(node, "filename").as_string();
        source->filterType = get_member(node, "filterType").as_string();
        result = std::move(source);
    }

    return result;
}

bool ParseBsdf(pugi::xml_node node, std::shared_ptr<MitsubaBsdf>& obj, std::unordered_map<std::string, std::shared_ptr<MitsubaObject>> &objs) {
    std::string bsdfType = node.attribute("type").as_string();
    if (bsdfType == "dielectric") {
        auto bsdf = std::make_shared<MitsubaBsdfDielectric>();
        bsdf->type = MitsubaBsdf::Type::Dielectric;
        bsdf->intIOR = get_member(node, "intIOR").as_float();
        bsdf->extIOR = get_member(node, "extIOR").as_float();
        obj = std::move(bsdf);
    } else if (bsdfType == "twosided") {
        auto bsdf = std::make_shared<MitsubaBsdfTwoSided>();
        bsdf->type = MitsubaBsdf::Type::Twosided;
        if (!ParseBsdf(node.child("bsdf"), bsdf->front, objs)) {
            return false;
        }
        obj = std::move(bsdf);
    } else if(bsdfType == "diffuse") {
        auto bsdf = std::make_shared<MitsubaBsdfDiffuse>();
        bsdf->type = MitsubaBsdf::Type::Diffuse;
        bsdf->reflectance = ParseColorSource(node.find_child_by_attribute("name", "reflectance"));
        obj = std::move(bsdf);
    } else if (bsdfType == "roughplastic") {
        auto bsdf = std::make_shared<MitsubaBsdfRoughPlastic>();
        bsdf->type = MitsubaBsdf::Type::RoughPlastic;
        bsdf->alpha = get_member(node, "alpha").as_float();
        bsdf->distribution = get_member(node, "distribution").as_string();
        bsdf->intIOR = get_member(node, "intIOR").as_float();
        bsdf->extIOR = get_member(node, "extIOR").as_float();
        bsdf->nonLinear = get_member(node, "nonlinear").as_bool();
        bsdf->diffuseReflectance = ParseColorSource(node.find_child_by_attribute("name", "diffuseReflectance"));
        obj = std::move(bsdf);
    } else if (bsdfType == "roughconductor") {
        auto bsdf = std::make_shared<MitsubaBsdfRoughConductor>();
        bsdf->type = MitsubaBsdf::Type::RoughConductor;
        bsdf->alpha = get_member(node, "alpha").as_float();
        bsdf->distribution = get_member(node, "distribution").as_string();
        bsdf->extETA = get_member(node, "extEta").as_float();
        bsdf->specularReflectance = ParseColorSource(node.find_child_by_attribute("name", "specularReflectance"));
        bsdf->eta = ParseColorSource(node.find_child_by_attribute("name", "eta"));
        bsdf->k = ParseColorSource(node.find_child_by_attribute("name", "k"));
        obj = std::move(bsdf);
    } else if (bsdfType == "conductor") {
        auto bsdf = std::make_shared<MitsubaBsdfConductor>();
        bsdf->type = MitsubaBsdf::Type::Conductor;
        bsdf->extETA = get_member(node, "extEta").as_float();
        bsdf->specularReflectance = ParseColorSource(node.find_child_by_attribute("name", "specularReflectance"));
        bsdf->eta = ParseColorSource(node.find_child_by_attribute("name", "eta"));
        bsdf->k = ParseColorSource(node.find_child_by_attribute("name", "k"));
        obj = std::move(bsdf);
    } else if (bsdfType == "bumpmap") {
        auto bsdf = std::make_shared<MitsubaBsdfBumpmap>();
        bsdf->type = MitsubaBsdf::Type::Twosided;
        if (!ParseBsdf(node.child("bsdf"), bsdf->bsdf, objs)) {
            return false;
        }
        bsdf->map = ParseColorSource(node.find_child_by_attribute("name", "map"));

        obj = std::move(bsdf);
    } else {
        std::cerr << "Unknow BSDF type: " << bsdfType << std::endl;
        return false;
    }

    std::string id = node.attribute("id").as_string();
    obj->id = id;
    obj->objType = MitsubaObject::Type::BSDF;
    objs.insert({ obj->id != "" ? obj->id : std::to_string(objs.size()), obj });
    return true;
}

bool ParseShape(pugi::xml_node node,
    std::unordered_map<std::string, std::shared_ptr<MitsubaObject>>& objs,
    std::shared_ptr<MitsubaShape>& obj) {
    std::string shapeType = node.attribute("type").as_string();
    if (shapeType == "obj") {
        auto shape = std::make_shared<MitsubaShapeObj>();
        shape->type = MitsubaShape::Type::Obj;
        shape->filename = get_member(node, "filename").as_string();
        shape->faceNormals = get_member(node, "faceNormals").as_bool();
        obj = std::move(shape);
    } else if (shapeType == "disk") {
        auto shape = std::make_shared<MitsubaShapeDisk>();
        shape->type = MitsubaShape::Type::Disk;
        obj = std::move(shape);
    } else if (shapeType == "rectangle") {
        auto shape = std::make_shared<MitsubaShapeRectangle>();
        shape->type = MitsubaShape::Type::Rectangle;
        obj = std::move(shape);
    } else if (shapeType == "cube") {
        auto shape = std::make_shared<MitsubaShapeCube>();
        shape->type = MitsubaShape::Type::Cube;
        obj = std::move(shape);
    } else if (shapeType == "sphere") {
        auto shape = std::make_shared<MitsubaShapeSphere>();
        shape->type = MitsubaShape::Type::Sphere;
        shape->radius = get_member(node, "radius").as_float();
        auto centerNode = node.find_child_by_attribute("name", "center");
        shape->center.x = centerNode.attribute("x").as_float();
        shape->center.y = centerNode.attribute("y").as_float();
        shape->center.z = centerNode.attribute("z").as_float();
        obj = std::move(shape);
    } else {
        std::cerr << "Unknow shape type: " << shapeType << std::endl;
        return false;
    }

    for (auto refNode : node.children("ref")) {
        auto refIt = objs.find(std::string(refNode.attribute("id").as_string()));
        if (refIt == objs.end()) {
            std::cerr << "Invalid reference to " << refNode.attribute("id").as_string() << std::endl;
            return false;
        }
        auto refObj = refIt->second;

        switch (refObj->objType) {
            case MitsubaObject::Type::BSDF:
                obj->material = std::static_pointer_cast<MitsubaBsdf>(refObj);
                break;
            default:
                std::cerr << "Invalid reference to " << refNode.attribute("id") << "of type " << (int)refObj->objType << std::endl;
                return false;
        }
    }

    auto emitterNode = node.child("emitter");
    if (emitterNode) {
        std::string emitterType = emitterNode.attribute("type").as_string();
        if (emitterType == "area") {
            auto emitter = std::make_shared<MitsubaEmitterArea>();
            emitter->radiance = ParseColorSource(emitterNode.find_child_by_attribute("name", "radiance"));
            obj->emitter = emitter;
        } else {
            std::cerr << "Unknow emitter type: " << emitterType << std::endl;
            return false;
        }
    } else {
        obj->emitter = nullptr;
    }

    auto bsdfNode = node.child("bdsf");
    if (bsdfNode) {
        if (!ParseBsdf(bsdfNode, obj->material, objs)) {
            return false;
        }
    }

    auto transformNode = node.child("transform");
    if (transformNode) {
        obj->transform = ParseMatrix(transformNode.child("matrix").attribute("value").as_string());
    } else {
        obj->transform = Matrix::Identity();
    }

    std::string id = node.attribute("id").as_string();
    obj->id = id;
    obj->objType = MitsubaObject::Type::Shape;
    objs.insert({ obj->id != "" ? obj->id : std::to_string(objs.size()), obj });
    return true;
}


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

        std::string typeString = integratorNode.attribute("type").as_string();
        if (typeString == "volpath" || typeString == "path") {
            auto integrator = std::make_unique<MitsubaIntegratorVolPath>();
            integrator->type = MitsubaIntegrator::Type::VolPath;
            integrator->maxDepth = get_member(integratorNode, "maxDepth").as_int();
            integrator->strictNormals = get_member(integratorNode, "strictNormals").as_bool();
            scene.integrator = std::move(integrator);
        } else {
            std::cerr << "Unknow integrator type: " << typeString << std::endl;
            return false;
        }
    }

    {
        auto sensorNode = sceneNode.child("sensor");
        if (!sensorNode) {
            std::cerr << "No sensor node in Mitsuba scene file.\n";
            return false;
        }

        std::string typeString = sensorNode.attribute("type").as_string();
        if (typeString == "perspective") {
            auto sensor = std::make_unique<MitsubaSensorPerspective>();
            sensor->type = MitsubaSensor::Type::Perspective;
            sensor->fov = get_member(sensorNode, "fov").as_float();
            scene.sensor = std::move(sensor);
        } else {
            std::cerr << "Unknow sensor type: " << typeString << std::endl;
            return false;
        }

        auto transformNode = sensorNode.child("transform");
        scene.sensor->transform = ParseMatrix(transformNode.child("matrix").attribute("value").as_string());

        {
            auto samplerNode = sensorNode.child("sampler");
            if (!samplerNode) {
                std::cerr << "Sensor has no sampler!\n";
                return false;
            }

            std::string samplerTypeString = samplerNode.attribute("type").as_string();
            if (samplerTypeString == "sobol") {
                auto sampler = std::make_unique<MitsubaSamplerSobol>();
                sampler->type = MitsubaSampler::Type::Sobol;
                scene.sensor->sampler = std::move(sampler);
            } else {
                std::cerr << "Unknow sampler type: " << samplerTypeString << std::endl;
                return false;
            }

            scene.sensor->sampler->sampleCount = get_member(samplerNode, "sampleCount").as_int();
        }

        {
            auto filmNode = sensorNode.child("film");
            if (!filmNode) {
                std::cerr << "Sensor has no film!\n";
                return false;
            }

            std::string filmTypeString = filmNode.attribute("type").as_string();
            if (filmTypeString == "ldrfilm") {
                auto film = std::make_unique<MitsubaFilmLDR>();
                film->type = MitsubaFilm::Type::LDR;
                scene.sensor->film = std::move(film);
            } else {
                std::cerr << "Unknow film type: " << filmTypeString << std::endl;
                return false;
            }

            scene.sensor->film->width = get_member(filmNode, "width").as_int();
            scene.sensor->film->height = get_member(filmNode, "height").as_int();
            scene.sensor->film->fileFormat = get_member(filmNode, "fileFormat").as_string();
            scene.sensor->film->pixelFormat = get_member(filmNode, "pixelFormat").as_string();
            scene.sensor->film->gamma = get_member(filmNode, "gamma").as_float();
            scene.sensor->film->banner = get_member(filmNode, "banner").as_bool();

            {
                auto filterNode = filmNode.child("rfilter");
                if (!filterNode) {
                    std::cerr << "Sensor has no film!\n";
                    return false;
                }

                std::string filterTypeString = filterNode.attribute("type").as_string();
                if (filterTypeString == "tent") {
                    auto filter = std::make_unique<MitsubaRFilterTent>();
                    filter->type = MitsubaRFilter::Type::Tent;
                    scene.sensor->film->rFilter = std::move(filter);
                } else {
                    std::cerr << "Unknow filter type: " << filterTypeString << std::endl;
                    return false;
                }
            }
        }
    }

    for (auto bsdfNode : sceneNode.children("bsdf")) {
        std::shared_ptr<MitsubaBsdf> bsdf;
        if (!ParseBsdf(bsdfNode, bsdf, scene.objects)) return false;
    }

    for (auto shapeNode : sceneNode.children("shape")) {
        std::shared_ptr<MitsubaShape> shape;
        if (!ParseShape(shapeNode, scene.objects, shape)) return false;
    }

    return true;
}

Material* GetMaterialFromBsdf(MitsubaBsdf* bsdf) {
    switch (bsdf->type) {
        case MitsubaBsdf::Type::Diffuse:
        {
            auto diffuseMat = (MitsubaBsdfDiffuse*)bsdf;
            auto reflectance = diffuseMat->reflectance.get();
            Color col;
            if (reflectance->type == MitsubaColorSource::Type::RGB) {
                col = ((MitsubaColorSourceRGB*)reflectance)->color;
            } else {
                col = Color(0.7, 0.7, 0.7);
                std::cerr << "Color source is a texture, but textures are not yet supported.\n";
            }

            return new SpecularMaterial(col, 1.0f, 0.0f, 0.0f, 0.0f);
        }
        case MitsubaBsdf::Type::Twosided: {
            return GetMaterialFromBsdf(((MitsubaBsdfTwoSided*)bsdf)->front.get());
        }
        case MitsubaBsdf::Type::Bumpmap: {
            return GetMaterialFromBsdf(((MitsubaBsdfBumpmap*)bsdf)->bsdf.get());
        }
        case MitsubaBsdf::Type::RoughConductor: {
            auto conductorMat = (MitsubaBsdfRoughConductor*)bsdf;

            auto reflectance = conductorMat->specularReflectance.get();
            Color col;
            if (reflectance->type == MitsubaColorSource::Type::RGB) {
                col = ((MitsubaColorSourceRGB*)reflectance)->color;
            } else {
                col = Color(0.7, 0.7, 0.7);
                std::cerr << "Color source is a texture, but textures are not yet supported.\n";
            }

            return new SpecularMaterial(col, 0, 1, 0, conductorMat->alpha);
        }
        case MitsubaBsdf::Type::Conductor: {
            auto conductorMat = (MitsubaBsdfConductor*)bsdf;

            auto reflectance = conductorMat->specularReflectance.get();
            Color col;
            if (reflectance->type == MitsubaColorSource::Type::RGB) {
                col = ((MitsubaColorSourceRGB*)reflectance)->color;
            } else {
                col = Color(0.7, 0.7, 0.7);
                std::cerr << "Color source is a texture, but textures are not yet supported.\n";
            }

            return new SpecularMaterial(col, 0, 1, 0, 0);
        }
        case MitsubaBsdf::Type::RoughPlastic: {
            auto plasticMat = (MitsubaBsdfRoughPlastic*)bsdf;

            auto reflectance = plasticMat->diffuseReflectance.get();
            Color col;
            if (reflectance->type == MitsubaColorSource::Type::RGB) {
                col = ((MitsubaColorSourceRGB*)reflectance)->color;
            } else {
                col = Color(0.7, 0.7, 0.7);
                std::cerr << "Color source is a texture, but textures are not yet supported.\n";
            }

            return new SpecularMaterial(col, 0.2f, 0.8f, 0.0f, plasticMat->alpha);
        }
        case MitsubaBsdf::Type::Dielectric: {
            auto dielectricMat = (MitsubaBsdfDielectric*)bsdf;
            return new SpecularMaterial(Color(1, 1, 1), 0, 0.1f, 0.9f, 0.0001f);
        }
        default: {
            return new DiffuseMaterial(Color(0.5, 0.5, 0.5));
        }
    }
    return nullptr;
}


bool LoadMitsuba(std::istream& sceneStream, std::string sceneFileName, std::vector<BaseObject *> &loadedObjects, Camera **loadedCamera) {
    MitsubaScene scene;
    if (!ParseMitsuba(sceneFileName, scene)) return false;

    switch (scene.sensor->type) {
        case MitsubaSensor::Type::Perspective:
        {
            auto sensor = (MitsubaSensorPerspective*)scene.sensor.get();
            *loadedCamera = new PhysicallyBasedCamera(0, 0, XMConvertToRadians(sensor->fov));
            (*loadedCamera)->SetViewMatrix( Matrix::CreateFromAxisAngle(Vector3(0,1,0), XM_PI) * sensor->transform);
            break;
        }
        default:
            std::cerr << "Unsupported sensor type: " << (int)scene.sensor->type << std::endl;
            return false;
    }


    std::string sceneFileFolder =
        sceneFileName.substr(0, sceneFileName.find_last_of("\\/"));

    for (const auto& obj : scene.objects) {
        if (obj.second->objType != MitsubaObject::Type::Shape) continue;
        auto shape = (MitsubaShape*)obj.second.get();

        RenderObject* renderObj;

        switch (shape->type) {
            case MitsubaShape::Type::Obj: {
                auto objShape = (MitsubaShapeObj*)shape;

                auto meshFile = sceneFileFolder + "\\" + objShape->filename;

                std::vector<Triangle> tris;
                std::vector<Vector3> verts;
                std::vector<Vector3> normals;
                std::vector<Vector2> uvs;
                bool smooth = !objShape->faceNormals;

                if (!LoadObj(meshFile, tris, verts, normals, uvs, smooth)) {
                    std::cerr << "Could not load object at " << meshFile << std::endl;
                    return false;
                }

                renderObj = new Mesh(Vector3(), tris, verts, normals, uvs, smooth);
                break;
            }
            case MitsubaShape::Type::Disk: {
                //TODO: Create proper disk objects
                renderObj = new Sphere(Vector3(0, 0, 0), 1.0f);
                break;
            }
            case MitsubaShape::Type::Rectangle: {
                renderObj = new Box(Vector3(0, 0, 0), Vector3(1.0f, 1.0f, 0.05f));
                break;
            }
            case MitsubaShape::Type::Cube: {
                renderObj = new Box(Vector3(0, 0, 0), Vector3(1.0f, 1.0f, 1.0f));
                break;
            }
            case MitsubaShape::Type::Sphere: {
                auto sphereShape = (MitsubaShapeSphere*)shape;
                renderObj = new Sphere(sphereShape->center, sphereShape->radius);
                break;
            }
        }

        Vector3 pos;
        Quaternion rot;
        Vector3 scale;

        shape->transform.Decompose(scale, rot, pos);
        renderObj->SetPosition(pos);
        renderObj->SetRotation(rot);
        renderObj->SetScale(scale);

        if (shape->emitter) {
            auto source = shape->emitter->radiance.get();
            if (source->type == MitsubaColorSource::Type::RGB) {
                renderObj->SetMaterial(new EmissionMaterial(((MitsubaColorSourceRGB*)source)->color));
            } else {
                renderObj->SetMaterial(new EmissionMaterial(Color(1, 1, 1)));
                std::cerr << "Emitter color source is a texture, but textures are not yet supported.\n";
            }
        } else if (shape->material) {
            renderObj->SetMaterial(GetMaterialFromBsdf(shape->material.get()));
        } else {
            renderObj->SetMaterial(new EmissionMaterial(Color(float(rand())/RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX)));
            std::cerr << "Object " << shape->id << "doesn't have a material assigned, using default diffuse.\n";
        }

        loadedObjects.push_back(renderObj);
    }

    return true;
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