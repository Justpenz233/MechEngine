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

using namespace luisa;
using namespace luisa::compute;

class ENGINE_API LightSceneProxy: public SceneProxy
{
public:
	explicit LightSceneProxy(GpuScene& InScene) noexcept;

	/**
	 * Add a new light to the scene and bind the corresponding transform
	 * @param InLight LightComponent to add
	 * @param InstanceId InstanceId of the light in the scene
	 */
	uint AddLight(LightComponent* InLight, uint InstanceId);

	/**
	 * Update the light data in the scene
	 * @param InLight LightComponent to update
	 * @param LightId light id in the scene
	 */
	void UpdateLight(LightComponent* InLight, uint LightId, uint InstanceId);

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
		return IdCounter;
	}

	[[nodiscard]] FORCEINLINE Var<light_data> get_light_data(const UInt& light_id) const
	{
		return bindelss_buffer<light_data>(bindless_id)->read(light_id);
	}

	Polymorphic<light_base> light_virtual_call;

public:

	[[nodiscard]] light_li_sample sample_li(const UInt& light_id, const Float3& x, const Float2& u) const;

	[[nodiscard]] std::pair<Float3, Float> l_i(const UInt& light_id, const Float3& x, const Float3& p_l) const;


protected:
	light_data GetFlatLightData(LightComponent* InLight) const;


	LightSceneProxy(const LightSceneProxy&) = delete;
	LightSceneProxy& operator=(const LightSceneProxy&) = delete;
	LightSceneProxy(LightSceneProxy&&) = delete;

	//Light collection
	uint IdCounter = 0;
	vector<light_data> LightDatas;
	uint bindless_id;
	BufferView<light_data> light_buffer;

	uint const_light_tag;
	uint point_light_tag;
	uint rectangle_light_tag;

	bool bDirty = false;
};
}