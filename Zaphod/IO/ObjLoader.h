#pragma once
#include <string>
#include <vector>
#include "../SimpleMath.h"

struct Triangle;

bool LoadObj(const std::string &_file, std::vector<Triangle>& _tris, std::vector<DirectX::SimpleMath::Vector3>& _verts, std::vector<DirectX::SimpleMath::Vector3>& _normals, std::vector<DirectX::SimpleMath::Vector2>& _uvs,
  bool &smooth);