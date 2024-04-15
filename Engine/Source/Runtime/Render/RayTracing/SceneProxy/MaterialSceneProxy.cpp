//
// Created by MarvelLi on 2024/4/11.
//

#include "MaterialSceneProxy.h"

#include "Render/Core/blinn_phong_material.h"
#include "Render/Core/TypeConvertion.h"
#include "Render/RayTracing/RayTracingScene.h"

Rendering::MaterialSceneProxy::MaterialSceneProxy(RayTracingScene& InScene)
:SceneProxy(InScene)
{
	material_data_buffer = Scene.RegisterBuffer<materialData>(MaxMaterials);

	MaterialModeTagMap[BlinnPhong] = material_tags.create<blinn_phong_material>();
}

uint Rendering::MaterialSceneProxy::AddMaterial(Material* InMaterial)
{
	ASSERTMSG(InMaterial != nullptr, "Material is nullptr!");
	if (MaterialIDMap.contains(InMaterial)) return MaterialIDMap[InMaterial];
	uint tag;
	if (MaterialModeTagMap.contains(InMaterial->Mode))
	{
		tag = MaterialModeTagMap[InMaterial->Mode];
	}
	else
	{
		ASSERTMSG(false, "Material mode is not supported yet!");
	}
	bNeedUpdate = true;
	uint id = MaterialDataVector.size();
	MaterialIDMap[InMaterial] = id;
	MaterialDataVector.emplace_back(tag, InMaterial);
	return id;
}

void Rendering::MaterialSceneProxy::UpdateMaterial(Material* InMaterial)
{
	ASSERTMSG(InMaterial != nullptr, "Material is nullptr!");
	ASSERTMSG(MaterialIDMap.contains(InMaterial), "Material is not in the map, add to scene first!");
	bNeedUpdate = true;
	uint ID = MaterialIDMap[InMaterial];
	uint Tag = MaterialDataVector[ID].material_type;
	MaterialDataVector[ID] = materialData(Tag, InMaterial);
}

void Rendering::MaterialSceneProxy::UploadDirtyData(Stream& stream)
{
	if (!bNeedUpdate) return;
	bNeedUpdate = false;
	stream << material_data_buffer.subview(0, MaterialDataVector.size())
	.copy_from(MaterialDataVector.data());
}