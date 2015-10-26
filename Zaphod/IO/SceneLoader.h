#pragma once
#include <string>
#include <vector>

class Camera;
class BaseObject;

bool LoadScene(const std::string &sceneFileName,
               std::vector<BaseObject *> &loadedObjects, Camera **loadedCamera);