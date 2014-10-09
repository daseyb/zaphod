#include "ObjLoader.h"
#include <fstream>
#include <sstream>
#include "Geometry/Triangle.h"

using namespace DirectX::SimpleMath;

bool LoadObj(const std::string& _file, std::vector<Triangle>& _outTris, bool& smooth) {
	std::ifstream file;
	file.open(_file);
	if(!file.is_open()) 
	std::vector<unsigned int> vertexIndices, normalIndices, uvIndices;
	std::vector<Vector3> tempVertices;
	std::vector<Vector3> tempNormals;
	std::vector<Vector2> tempUvs;

	while(!file.eof()) {
		std::string line;
		std::getline(file, line);

		std::stringstream lineStream(line);
		std::string lineHeader;
		lineStream >> lineHeader;
		
		if(lineHeader == "v") {
			Vector3 newVec;
			lineStream >> newVec.x >> newVec.y >> newVec.z;
			tempVertices.push_back(newVec);
		} else if(lineHeader == "vn") {
			Vector3 newNorm;
			lineStream >> newNorm.x >> newNorm.y >> newNorm.z;
			tempNormals.push_back(newNorm);
		} else if(lineHeader == "vt") {
			Vector2 newUv;
			lineStream >> newUv.x >> newUv.y;
			tempUvs.push_back(newUv);
		} else if(lineHeader == "f") {
			unsigned int vertexIndex[3];
			unsigned int normalIndex[3]; 
			unsigned int uvIndex[3];
			
			std::string v1, v2, v3;
			lineStream >> v1 >> v2 >> v3;

			sscanf(v1.c_str(), "%d/%d/%d", &vertexIndex[0], &uvIndex[0], &normalIndex[0]);
			sscanf(v2.c_str(), "%d/%d/%d", &vertexIndex[1], &uvIndex[1], &normalIndex[1]);
			sscanf(v3.c_str(), "%d/%d/%d", &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

			vertexIndices.push_back(vertexIndex[0] - 1);
			vertexIndices.push_back(vertexIndex[1] - 1);
			vertexIndices.push_back(vertexIndex[2] - 1);
			normalIndices.push_back(normalIndex[0] - 1);
			normalIndices.push_back(normalIndex[1] - 1);
			normalIndices.push_back(normalIndex[2] - 1);
			uvIndices    .push_back(uvIndex[0] - 1);
			uvIndices    .push_back(uvIndex[1] - 1);
			uvIndices    .push_back(uvIndex[2] - 1);
		} else if ( lineHeader == "s" ) {
			std::string smoothStr;
			lineStream >> smoothStr;
			smooth = smoothStr == "1";
		}
	}

	_outTris.reserve(vertexIndices.size()/3);
	for(int i = 0; i < vertexIndices.size()/3; i++) {
		Vertex v1, v2, v3;
		v1.Position = tempVertices[vertexIndices[i*3 + 0]];
		v2.Position = tempVertices[vertexIndices[i*3 + 1]];
		v3.Position = tempVertices[vertexIndices[i*3 + 2]];

		v1.Normal	= tempNormals[normalIndices[i*3 + 0]];
		v2.Normal	= tempNormals[normalIndices[i*3 + 1]];
		v3.Normal	= tempNormals[normalIndices[i*3 + 2]];

		v1.UV		= tempUvs[uvIndices[i*3 + 0]];
		v2.UV		= tempUvs[uvIndices[i*3 + 1]];
		v3.UV		= tempUvs[uvIndices[i*3 + 2]];

		_outTris.push_back(Triangle(v1, v2, v3));
	}
	return true;
}
