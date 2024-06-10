//
// Created by MarvelLi on 2024/4/11.
//

#pragma once
#include <luisa/dsl/polymorphic.h>
#include "SceneProxy.h"
#include "Log/Log.h"
#include "Materials/Material.h"
#include "Render/material/material_base.h"

namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;
class ENGINE_API MaterialSceneProxy : public SceneProxy
{
public:
	MaterialSceneProxy(RayTracingScene& InScene);

	/**
	* Add a material to the scene, return the material id
	* if exist, return the material id
	* @return the material id
	*/
	uint AddMaterial(Material* InMaterial);

	/**
	* Update the material data in the scene
	* @param InMaterial Material to update
	*/
	void UpdateMaterial(Material* InMaterial);

	FORCEINLINE bool IsMaterialUploaded(Material* InMaterial) const;

	FORCEINLINE uint GetMaterialID(Material* InMaterial);

	virtual void UploadDirtyData(Stream& stream) override;

public:
	Polymorphic<material_base> material_virtual_call;

	Var<materialData> get_material_data(Expr<uint> material_index) const
	{
		return material_data_buffer->read(material_index);
	}

protected:
	static constexpr uint MaxMaterials = 8192;
	vector<materialData> MaterialDataVector;
	BufferView<materialData> material_data_buffer;

	THashMap<class Material*, uint> MaterialIDMap;

	THashMap<MaterialMode, uint> MaterialModeTagMap;

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
