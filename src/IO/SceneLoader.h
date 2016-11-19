#pragma once
#include <string>
#include <vector>

class Camera;
class BaseObject;

struct ObjectData {
	std::string MaterialName;
	BaseObject *Object;
};

bool LoadScene(const std::string &sceneFileName,
               std::vector<BaseObject *> &loadedObjects, Camera **loadedCamera);