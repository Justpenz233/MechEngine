//
// Created by MarvelLi on 2024/5/7.
//

#include "Material.h"
#include "Render/GPUSceneInterface.h"

Material::Material()
{
	RegisterMaterial();
}

void Material::RegisterMaterial() {
	GWorld->GetScene()->AddMaterial(this);
}

void Material::UpdateMaterial() {
	GWorld->GetScene()->UpdateMaterial(this);
}