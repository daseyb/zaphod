#pragma once
#include <string>

class BaseObject;

bool LoadAbc(std::string abcFile, BaseObject** result);