#pragma once
#include <string>
#include <vector>

class BaseObject;
struct ObjectData;

bool LoadAbc(std::string abcFile, BaseObject **root, std::vector<ObjectData*>& resultData);