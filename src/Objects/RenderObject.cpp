#include "RenderObject.h"
#include "../Rendering/Materials/Material.h"
#include "../Rendering/Materials/EmissionMaterial.h"

using namespace DirectX::SimpleMath;

RenderObject::RenderObject(BaseObject *parent) : BaseObject(parent) {
	m_Material = nullptr;
}

Material *RenderObject::GetMaterial() const { return m_Material.get(); }

void RenderObject::SetMaterial(Material *_mat) {
  m_Material.reset(_mat->Copy());
}
