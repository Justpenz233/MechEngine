//
// Created by MarvelLi on 2024/4/11.
//

#include "MaterialSceneProxy.h"
#include "Render/material/shader_base.h"
#include "Render/material/blinn_phong_material.h"
#include "Render/material/disney_material.h"
#include "Render/PipeLine/GpuScene.h"

Rendering::MaterialSceneProxy::MaterialSceneProxy(GpuScene& InScene)
:SceneProxy(InScene)
{
	material_data_buffer = Scene.RegisterBuffer<material_data>(MaxMaterials);

	auto disney_shader_id = shader_call.create<disney_material>();
	auto blinn_phong_id = shader_call.create<blinn_phong_material>();

	ASSERT(disney_shader_id == 0);
	ASSERT(blinn_phong_id == 1);
}

uint Rendering::MaterialSceneProxy::AddMaterial(Material* InMaterial)
{
	ASSERTMSG(InMaterial != nullptr, "Material is nullptr!");
	if (MaterialIDMap.contains(InMaterial))
		return MaterialIDMap[InMaterial];

	if(!ShaderValid(InMaterial->ShaderId))
	{
		LOG_ERROR("Shader id {} is invalid!", InMaterial->ShaderId);
		return -1;
	}
	bNeedUpdate = true;
	uint id = MaterialDataVector.size();
	MaterialIDMap[InMaterial] = id;
	MaterialDataVector.emplace_back(InMaterial->ShaderId, InMaterial);
	return id;
}

void Rendering::MaterialSceneProxy::UpdateMaterial(Material* InMaterial)
{
	ASSERTMSG(InMaterial != nullptr, "Material is nullptr!");
	if(!MaterialIDMap.contains(InMaterial)) return;
	bNeedUpdate = true;
	uint ID = MaterialIDMap[InMaterial];
	uint ShaderID = InMaterial->ShaderId;
	MaterialDataVector[ID] = material_data(ShaderID, InMaterial);
}

void Rendering::MaterialSceneProxy::UploadDirtyData(Stream& stream)
{
	if (!bNeedUpdate)
		return;
	bNeedUpdate = false;
	stream << material_data_buffer.subview(0, MaterialDataVector.size())
				  .copy_from(MaterialDataVector.data());
}

uint Rendering::MaterialSceneProxy::RegisterShader(luisa::unique_ptr<shader_base>&& Shader)
{
	return shader_call.emplace(std::move(Shader));
}