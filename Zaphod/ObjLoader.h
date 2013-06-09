#pragma once
#include <string>
#include <vector>
#include "SimpleMath.h"

class Triangle;

bool LoadObj(const std::string& _file, std::vector<Triangle>& _outTris, bool& _smooth);