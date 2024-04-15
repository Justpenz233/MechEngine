//
// Created by MarvelLi on 2024/4/11.
//

#pragma once
#include <luisa/dsl/polymorphic.h>
#include "SceneProxy.h"
#include "Log/Log.h"
#include "Materials/Material.h"
#include "Render/Core/material_base.h"

namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;
class MaterialSceneProxy : public SceneProxy
{
public:
	MaterialSceneProxy(RayTracingScene& InScene);

	uint AddMaterial(Material* InMaterial);

	void UpdateMaterial(Material* InMaterial);

	FORCEINLINE bool IsMaterialUploaded(Material* InMaterial) const;

	FORCEINLINE uint GetMaterialID(Material* InMaterial);

	virtual void UploadDirtyData(Stream& stream) override;

protected:
	static constexpr uint MaxMaterials = 1024;
	vector<materialData> MaterialDataVector;
	BufferView<materialData> material_data_buffer;

	THashMap<class Material*, uint> MaterialIDMap;

	THashMap<MaterialMode, uint> MaterialModeTagMap;
	Polymorphic<material_base> material_tags;

	bool bNeedUpdate = false;

};

FORCEINLINE bool MaterialSceneProxy::IsMaterialUploaded(Material* InMaterial) const
{
	return MaterialIDMap.contains(InMaterial);
}

FORCEINLINE uint MaterialSceneProxy::GetMaterialID(Material* InMaterial)
{
	ASSERTMSG(MaterialIDMap.contains(InMaterial), "Material is not in the map, add to scene first!");
	return MaterialIDMap[InMaterial];
}
}
