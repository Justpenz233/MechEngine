//
// Created by MarvelLi on 2024/4/5.
//

#pragma once
#include "SceneProxy.h"
#include "Misc/Platform.h"
#include "luisa/luisa-compute.h"


namespace MechEngine::Rendering
{
class RayTracingScene;

// data structure for all the lights, some data maybe not used in different light types
struct lightData
{
	luisa::compute::uint transform_id;

	float intensity;

	luisa::compute::float3 linear_color;

	int samples_per_pixel;
};

}

LUISA_STRUCT(MechEngine::Rendering::lightData, transform_id, intensity, linear_color, samples_per_pixel)
{};

class LightComponent;
namespace MechEngine::Rendering
{

using namespace  luisa;
using namespace luisa::compute;

class LightSceneProxy: public SceneProxy
{
public:
	LightSceneProxy(RayTracingScene& InScene) noexcept;

	void AddLight(LightComponent* InLight, uint InTransformID);

	void UpdateLight(LightComponent* InLight);

	virtual void UploadDirtyData(Stream& stream) override;

	[[nodiscard]] FORCEINLINE uint LightCount() const
	{
		return id;
	}

	[[nodiscard]] FORCEINLINE Var<lightData> get_light_data(const UInt& light_id) const
	{
		return light_buffer->read(light_id);
	}

protected:
	//Light collection
	uint id = 0;
	static constexpr auto light_max_number = 256u;
	vector<lightData> LightDatas;
	BufferView<lightData> light_buffer;
	// component to [lightid, transformid]
	map<LightComponent*, std::pair<uint, uint>> LightIndexMap;
	set<LightComponent*> DirtyLights;
};
}