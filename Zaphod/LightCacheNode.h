#pragma once
#include "SimpleMath.h"

class LightCacheNode
{
private:
	int storedColors;
	DirectX::SimpleMath::Color color;
	LightCacheNode* childs[8];
	DirectX::BoundingBox childBounds[8];
	float volume;
	
	DirectX::SimpleMath::Vector3 GetChildPos(int _index, DirectX::BoundingBox _bounds);
	int GetChildIndex(DirectX::SimpleMath::Vector3 _pos);
public:
	LightCacheNode(DirectX::BoundingBox _bounds);
	void AddPoint(DirectX::SimpleMath::Vector3 _point, DirectX::SimpleMath::Color _color, float _minSize);
	bool Lookup(DirectX::SimpleMath::Vector3 _point, float _minSize, DirectX::SimpleMath::Color* _outCol, int* _storedCount);
};

