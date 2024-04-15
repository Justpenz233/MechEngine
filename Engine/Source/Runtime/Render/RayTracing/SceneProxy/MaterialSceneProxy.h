//
// Created by MarvelLi on 2024/4/11.
//

#pragma once
#include <luisa/dsl/polymorphic.h>
#include "SceneProxy.h"
#include "Materials/Material.h"
#include "Render/Core/material_base.h"

namespace MechEngine::Rendering
{
using namespace  luisa;
using namespace luisa::compute;
class MaterialSceneProxy : public SceneProxy
{
public:
	MaterialSceneProxy(RayTracingScene& InScene);

	void AddMaterial(Material* InMaterial);
	void UpdateMaterial(Material* InMaterial);
	void RemoveMaterial(Material* InMaterial);
public:


protected:
	static constexpr uint MaxMaterials = 1024;
	vector<materialData> material_data_vector;
	BufferView<materialData> material_data_buffer;

	Polymorphic<material_base> material_tags;

};

}
