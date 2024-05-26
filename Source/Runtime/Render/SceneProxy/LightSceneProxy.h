//
// Created by MarvelLi on 2024/4/5.
//

#pragma once
#include "SceneProxy.h"
#include "Misc/Platform.h"
#include "Render/light/light_base.h"
#include "luisa/luisa-compute.h"

class LightComponent;
namespace MechEngine::Rendering
{

using namespace  luisa;
using namespace luisa::compute;

class LightSceneProxy: public SceneProxy
{
public:
	explicit LightSceneProxy(RayTracingScene& InScene) noexcept;

	/**
	 * Add a new light to the scene and bind the corresponding transform
	 * @param InLight LightComponent to add
	 * @param InTransformID Corresponding transform id
	 */
	void AddLight(LightComponent* InLight, uint InTransformID);

	/**
	 * Update the light data in the scene
	 * @param InLight LightComponent to update
	 */
	void UpdateLight(LightComponent* InLight);

	/**
	 * Get the light type tag uint
	 * Return ~0u if the light type is not supported
	 * @param InLight
	 * @return light type tag
	 */
	uint GetLightTypeTag(LightComponent* InLight) const;

	virtual void UploadDirtyData(Stream& stream) override;

	[[nodiscard]] FORCEINLINE uint LightCount() const
	{
		return id;
	}

	[[nodiscard]] FORCEINLINE Var<light_data> get_light_data(const UInt& light_id) const
	{
		return bindelss_buffer<light_data>(bindless_id)->read(light_id);
	}

	Polymorphic<light_base> light_virtual_call;
protected:
	//Light collection
	uint id = 0;
	static constexpr auto light_max_number = 256u;
	vector<light_data> LightDatas;
	uint bindless_id;
	BufferView<light_data> light_buffer;

	uint const_light_tag;
	uint point_light_tag;
	uint directional_light_tag;

	// component to [lightid, transformid]
	map<LightComponent*, std::pair<uint, uint>> LightIndexMap;
	set<LightComponent*> DirtyLights;
};
}