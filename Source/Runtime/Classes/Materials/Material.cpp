//
// Created by MarvelLi on 2024/5/7.
//

#include "Material.h"
#include "Render/GPUSceneInterface.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"

Material::Material()
{
	RegisterMaterial();
}

void Material::RegisterMaterial() {
	GWorld->GetScene()->GetMaterialProxy()->AddMaterial(this);
}

void Material::UpdateMaterial() {
	GWorld->GetScene()->GetMaterialProxy()->UpdateMaterial(this);
}