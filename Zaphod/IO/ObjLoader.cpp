#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include "../Geometry/Triangle.h"

using namespace DirectX::SimpleMath;

bool LoadObj(const std::string &_file, std::vector<Triangle>& _tris, std::vector<Vector3>& _verts, std::vector<Vector3>& _normals, std::vector<Vector2>& _uvs,
             bool &smooth) {
  std::ifstream file;
  file.open(_file);

  if (!file.is_open()) {
    throw "Could not open file " + _file;
  }

  std::vector<size_t> vertexIndices, normalIndices, uvIndices;

  while (!file.eof()) {
    std::string line;
    std::getline(file, line);

    std::stringstream lineStream(line);
    std::string lineHeader;
    lineStream >> lineHeader;

    if (lineHeader == "v") {
      Vector3 newVec;
      lineStream >> newVec.x >> newVec.y >> newVec.z;
      _verts.push_back(newVec);
    } else if (lineHeader == "vn") {
      Vector3 newNorm;
      lineStream >> newNorm.x >> newNorm.y >> newNorm.z;
      _normals.push_back(newNorm);
    } else if (lineHeader == "vt") {
      Vector2 newUv;
      lineStream >> newUv.x >> newUv.y;
      _uvs.push_back(newUv);
    } else if (lineHeader == "f") {
      size_t vertexIndex[3];
      size_t normalIndex[3];
      size_t uvIndex[3];

      std::string v1, v2, v3;
      lineStream >> v1 >> v2 >> v3;

      if (v1.find('/') != std::string::npos) {
        sscanf(v1.c_str(), "%d/%d/%d", &vertexIndex[0], &uvIndex[0],
               &normalIndex[0]);
        sscanf(v2.c_str(), "%d/%d/%d", &vertexIndex[1], &uvIndex[1],
               &normalIndex[1]);
        sscanf(v3.c_str(), "%d/%d/%d", &vertexIndex[2], &uvIndex[2],
               &normalIndex[2]);
        vertexIndices.push_back(vertexIndex[0] - 1);
        vertexIndices.push_back(vertexIndex[1] - 1);
        vertexIndices.push_back(vertexIndex[2] - 1);
       
        normalIndices.push_back(normalIndex[0] - 1);
        normalIndices.push_back(normalIndex[1] - 1);
        normalIndices.push_back(normalIndex[2] - 1);
        
        uvIndices.push_back(uvIndex[0] - 1);
        uvIndices.push_back(uvIndex[1] - 1);
        uvIndices.push_back(uvIndex[2] - 1);
      } else {
        vertexIndex[0] = std::atoi(v1.c_str());
        vertexIndex[1] = std::atoi(v2.c_str());
        vertexIndex[2] = std::atoi(v3.c_str());

        vertexIndices.push_back(vertexIndex[0] - 1);
        vertexIndices.push_back(vertexIndex[1] - 1);
        vertexIndices.push_back(vertexIndex[2] - 1);
      }
    } else if (lineHeader == "s") {
      std::string smoothStr;
      lineStream >> smoothStr;
      smooth = smoothStr == "1";
    }
  }

  _tris.reserve(vertexIndices.size() / 3);
  for (size_t i = 0; i < vertexIndices.size() / 3; i++) {
    /*
    if (tempNormals.size() > 0) {
      v1.Normal = tempNormals[normalIndices[i * 3 + 0]];
      v2.Normal = tempNormals[normalIndices[i * 3 + 1]];
      v3.Normal = tempNormals[normalIndices[i * 3 + 2]];
    } else {
      Vector3 faceNormal =
          (v2.Position - v1.Position).Cross(v3.Position - v1.Position);
      faceNormal.Normalize();
      v1.Normal = v2.Normal = v3.Normal = faceNormal;
    }

    if (tempUvs.size() > 0) {
      v1.UV = tempUvs[uvIndices[i * 3 + 0]];
      v2.UV = tempUvs[uvIndices[i * 3 + 1]];
      v3.UV = tempUvs[uvIndices[i * 3 + 2]];
    } else {
      v1.UV = v2.UV = v3.UV = Vector2(0, 0);
    }*/

    Vector3 v1 = _verts[vertexIndices[i * 3 + 0]];
    Vector3 v2 = _verts[vertexIndices[i * 3 + 1]];
    Vector3 v3 = _verts[vertexIndices[i * 3 + 2]];

    const static Vector3 Zero = Vector3(0, 0, 0);
    // Test the plane of the triangle.
    Vector3 Normal =
        XMVector3Cross(v2 - v1, v3 - v1);
    // Assert that the triangle is not degenerate.
    if (XMVector3Equal(Normal, Zero)) {
      continue;
    }
    _tris.push_back(Triangle(vertexIndices[i * 3 + 2], vertexIndices[i * 3 + 1], vertexIndices[i * 3 + 0]));
  }
  return true;
}