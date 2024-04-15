//
// Created by MarvelLi on 2024/4/11.
//

#include "MaterialSceneProxy.h"

#include "Render/RayTracing/RayTracingScene.h"

Rendering::MaterialSceneProxy::MaterialSceneProxy(RayTracingScene& InScene)
:SceneProxy(InScene)
{
	material_data_buffer = Scene.RegisterBuffer<materialData>(MaxMaterials);
}

void Rendering::MaterialSceneProxy::AddMaterial(Material* InMaterial)
{

}

void Rendering::MaterialSceneProxy::UpdateMaterial(Material* InMaterial)
{

}

void Rendering::MaterialSceneProxy::RemoveMaterial(Material* InMaterial)
{

}