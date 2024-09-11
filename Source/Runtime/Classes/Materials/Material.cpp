//
// Created by MarvelLi on 2024/5/7.
//

#include "Material.h"
#include "Render/GPUSceneInterface.h"
#include "Render/SceneProxy/MaterialSceneProxy.h"
#include "Render/material/shader_base.h"

Material::Material()
{
	ShaderId = GWorld->GetScene()->GetMaterialProxy()->GetDefaultShaderId();
}

void Material::RegisterMaterial()
{
	GWorld->GetScene()->GetMaterialProxy()->AddMaterial(this);
}
uint Material::InnerRegisterShader(Rendering::shader_base* Shader)
{
	return GWorld->GetScene()->GetMaterialProxy()->RegisterShader(luisa::unique_ptr<Rendering::shader_base>(Shader));
}

void Material::UpdateMaterial() {
	GWorld->GetScene()->GetMaterialProxy()->UpdateMaterial(this);
}