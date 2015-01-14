#include "LightCache.h"
#include "LightCacheNode.h"

LightCache::LightCache(DirectX::BoundingBox _bounds) {
	root = new LightCacheNode(_bounds);
}

LightCache::~LightCache() {
	delete root;
}

void LightCache::AddPoint(DirectX::SimpleMath::Vector3 _point, DirectX::SimpleMath::Color _color){
	root->AddPoint(_point, _color, 0.01f);
}

bool LightCache::LookUp(DirectX::SimpleMath::Vector3 _point, DirectX::SimpleMath::Color* _outCol, int* _storedCount) {
	return root->Lookup(_point, 0.01f, _outCol, _storedCount);
}


