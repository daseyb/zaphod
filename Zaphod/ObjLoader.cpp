#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include "Geometry/Triangle.h"

using namespace DirectX::SimpleMath;

bool LoadObj(const std::string& _file, std::vector<Triangle>& _outTris, bool& smooth) {
	std::ifstream file;
	file.open(_file);

	if (!file.is_open()) {
		throw "Could not open file " + _file;
	}

	std::vector<unsigned int> vertexIndices, normalIndices, uvIndices;
	std::vector<Vector3> tempVertices;
	std::vector<Vector3> tempNormals;
	std::vector<Vector2> tempUvs;

	while (!file.eof()) {
		std::string line;
		std::getline(file, line);

		std::stringstream lineStream(line);
		std::string lineHeader;
		lineStream >> lineHeader;

		if (lineHeader == "v") {
			Vector3 newVec;
			lineStream >> newVec.x >> newVec.y >> newVec.z;
			tempVertices.push_back(newVec);
		}
		else if (lineHeader == "vn") {
			Vector3 newNorm;
			lineStream >> newNorm.x >> newNorm.y >> newNorm.z;
			tempNormals.push_back(newNorm);
		}
		else if (lineHeader == "vt") {
			Vector2 newUv;
			lineStream >> newUv.x >> newUv.y;
			tempUvs.push_back(newUv);
		}
		else if (lineHeader == "f") {
			unsigned int vertexIndex[3];
			unsigned int normalIndex[3];
			unsigned int uvIndex[3];

			std::string v1, v2, v3;
			lineStream >> v1 >> v2 >> v3;

			if (v1.find('/') != std::string::npos) {
				sscanf(v1.c_str(), "%d/%d/%d", &vertexIndex[0], &uvIndex[0], &normalIndex[0]);
				sscanf(v2.c_str(), "%d/%d/%d", &vertexIndex[1], &uvIndex[1], &normalIndex[1]);
				sscanf(v3.c_str(), "%d/%d/%d", &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				vertexIndices.push_back(vertexIndex[0] - 1);
				vertexIndices.push_back(vertexIndex[1] - 1);
				vertexIndices.push_back(vertexIndex[2] - 1);
				normalIndices.push_back(normalIndex[0] - 1);
				normalIndices.push_back(normalIndex[1] - 1);
				normalIndices.push_back(normalIndex[2] - 1);
				uvIndices.push_back(uvIndex[0] - 1);
				uvIndices.push_back(uvIndex[1] - 1);
				uvIndices.push_back(uvIndex[2] - 1);
			}
			else {
				vertexIndex[0] = std::atoi(v1.c_str());
				vertexIndex[1] = std::atoi(v2.c_str());
				vertexIndex[2] = std::atoi(v3.c_str());

				vertexIndices.push_back(vertexIndex[0] - 1);
				vertexIndices.push_back(vertexIndex[1] - 1);
				vertexIndices.push_back(vertexIndex[2] - 1);
			}
		}
		else if (lineHeader == "s") {
			std::string smoothStr;
			lineStream >> smoothStr;
			smooth = smoothStr == "1";
		}
	}

	_outTris.reserve(vertexIndices.size() / 3);
	for (int i = 0; i < vertexIndices.size() / 3; i++) {
		Vertex v1, v2, v3;
		v1.Position = tempVertices[vertexIndices[i * 3 + 0]];
		v2.Position = tempVertices[vertexIndices[i * 3 + 1]];
		v3.Position = tempVertices[vertexIndices[i * 3 + 2]];

		if (tempNormals.size() > 0) {
			v1.Normal = tempNormals[normalIndices[i * 3 + 0]];
			v2.Normal = tempNormals[normalIndices[i * 3 + 1]];
			v3.Normal = tempNormals[normalIndices[i * 3 + 2]];
		}
		else {
			Vector3 faceNormal = (v2.Position - v1.Position).Cross(v3.Position - v1.Position);
			faceNormal.Normalize();
			v1.Normal = v2.Normal = v3.Normal = faceNormal;
		}

		if (tempUvs.size() > 0) {
			v1.UV = tempUvs[uvIndices[i * 3 + 0]];
			v2.UV = tempUvs[uvIndices[i * 3 + 1]];
			v3.UV = tempUvs[uvIndices[i * 3 + 2]];
		}
		else {
			v1.UV = v2.UV = v3.UV = Vector2(0, 0);
		}

		const static Vector3 Zero = Vector3(0, 0, 0);
		// Test the plane of the triangle.
		Vector3 Normal = XMVector3Cross(v2.Position - v1.Position, v3.Position - v1.Position);
		// Assert that the triangle is not degenerate.
		if (XMVector3Equal(Normal, Zero)) {
			continue;
		}
		_outTris.push_back(Triangle(v1, v2, v3));
	}
	return true;
}