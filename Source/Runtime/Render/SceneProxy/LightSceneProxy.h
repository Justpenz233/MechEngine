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
	LightSceneProxy(RayTracingScene& InScene) noexcept;

	void AddLight(LightComponent* InLight, uint InTransformID);

	void UpdateLight(LightComponent* InLight);

	virtual void UploadDirtyData(Stream& stream) override;

	[[nodiscard]] FORCEINLINE uint LightCount() const
	{
		return id;
	}

	[[nodiscard]] FORCEINLINE Var<light_data> get_light_data(const UInt& light_id) const
	{
		return light_buffer->read(light_id);
	}

	Polymorphic<light_base> light_virtual_call;
protected:
	//Light collection
	uint id = 0;
	static constexpr auto light_max_number = 256u;
	vector<light_data> LightDatas;
	BufferView<light_data> light_buffer;


	uint point_light_tag;
	uint directional_light_tag;

	// component to [lightid, transformid]
	map<LightComponent*, std::pair<uint, uint>> LightIndexMap;
	set<LightComponent*> DirtyLights;
};
}