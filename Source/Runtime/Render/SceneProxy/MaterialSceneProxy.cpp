//
// Created by MarvelLi on 2024/4/11.
//

#include "MaterialSceneProxy.h"
#include "Render/material/shader_base.h"
#include "Render/material/blinn_phong_material.h"
#include "Render/material/disney_material.h"
#include "Render/PipeLine/GpuScene.h"

namespace MechEngine::Rendering
{
MaterialSceneProxy::MaterialSceneProxy(GpuScene& InScene)
:SceneProxy(InScene)
{
	material_data_buffer = Scene.RegisterBuffer<material_data>(MaxMaterials);

	auto disney_shader_id = shader_call.create<disney_material>();
	auto blinn_phong_id = shader_call.create<blinn_phong_material>();

	ASSERT(disney_shader_id == 0);
	ASSERT(blinn_phong_id == 1);
}

uint MaterialSceneProxy::AddMaterial(Material* InMaterial)
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

void MaterialSceneProxy::UpdateMaterial(Material* InMaterial)
{
	ASSERTMSG(InMaterial != nullptr, "Material is nullptr!");
	if(!MaterialIDMap.contains(InMaterial)) return;
	bNeedUpdate = true;
	uint ID = MaterialIDMap[InMaterial];
	uint ShaderID = InMaterial->ShaderId;
	MaterialDataVector[ID] = material_data(ShaderID, InMaterial);
}

void MaterialSceneProxy::UploadDirtyData(Stream& stream)
{
	if (!bNeedUpdate)
		return;
	bNeedUpdate = false;
	stream << material_data_buffer.subview(0, MaterialDataVector.size())
				  .copy_from(MaterialDataVector.data());
}

uint MaterialSceneProxy::RegisterShader(luisa::unique_ptr<shader_base>&& Shader)
{
	return shader_call.emplace(std::move(Shader));
}

std::pair<UInt, material_parameters> MaterialSceneProxy::get_material_parameters(const ray_intersection& intersection) const noexcept
{
	auto material_data = get_material_data(intersection.material_id);

	bxdf_context context{
		.intersection = intersection,
		.material_data = material_data,
	};

	material_parameters bxdf_parameters;
	shader_call.dispatch(
		material_data.shader_id, [&](const shader_base* material) {
			bxdf_parameters = material->calc_material_parameters(context);
		});

	return { material_data.shader_id, bxdf_parameters };
}

Float3 MaterialSceneProxy::brdf(const UInt& shader_id, const material_parameters& bxdf_parameters, const Float3& local_wo, const Float3& local_wi) const
{
	Float3 brdf;
	shader_call.dispatch(shader_id, [&](const shader_base* material) {
		 brdf = material->bxdf(bxdf_parameters, local_wo, local_wi);
	});
	return brdf;
}

std::pair<Float3, Float> MaterialSceneProxy::brdf_pdf(const UInt& shader_id, const material_parameters& bxdf_parameters, const Float3& local_wo, const Float3& local_wi) const
{
	Float3 brdf; Float pdf;
	shader_call.dispatch(shader_id, [&](const shader_base* material) {
		brdf = material->bxdf(bxdf_parameters, local_wo, local_wi);
		pdf = material->pdf(bxdf_parameters, local_wo, local_wi);
	});
	return { brdf, pdf };
}

std::tuple<Float3, Float3, Float> MaterialSceneProxy::sample_brdf
(const UInt& shader_id, const material_parameters& bxdf_parameters, const Float3& local_wo, const Float2& u) const
{
	Float3 local_wi, brdf; Float pdf;
	shader_call.dispatch(shader_id,
		[&](const shader_base* material) {
			std::tie(local_wi, pdf) = material->sample(bxdf_parameters, local_wo, u);
			brdf = material->bxdf(bxdf_parameters, local_wo, local_wi);
	});
	return { local_wi, brdf, pdf };
}

} // namespace MechEngine::Rendering