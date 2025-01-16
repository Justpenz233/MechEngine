//
// Created by MarvelLi on 2024/4/11.
//

#pragma once
#include <luisa/dsl/polymorphic.h>
#include "SceneProxy.h"
#include "Log/Log.h"
#include "Materials/Material.h"
#include "Render/material/shader_base.h"

namespace MechEngine::Rendering
{
using namespace luisa;
using namespace luisa::compute;
class ENGINE_API MaterialSceneProxy : public SceneProxy
{
public:
	MaterialSceneProxy(GpuScene& InScene);

	/**
	 * Add a material to the scene, return the material id
	 * if existed, return the material id
	 * @return the material id
	 */
	uint AddMaterial(Material* InMaterial);

	/**
	 * Update the material data in the scene
	 * @param InMaterial Material to update
	 */
	void UpdateMaterial(Material* InMaterial);

	/**
	 * Is material uploaded
	 * @param InMaterial Material to check
	 * @return true if the material is uploaded
	 */
	FORCEINLINE bool IsMaterialUploaded(Material* InMaterial) const;

	FORCEINLINE uint GetMaterialID(Material* InMaterial);

	/**
	 * Is shader id valid
	 * @param ShaderId Shader id
	 * @return true if the shader is valid
	 */
	FORCEINLINE bool ShaderValid(uint ShaderId) const;

	virtual void UploadDirtyData(Stream& stream) override;

public:
	/**
	 * Create a shader and return the pointer to the shader
	 * @tparam T Shader type
	 * @tparam Args Shader constructor arguments type
	 * @param args Shader constructor arguments
	 * @return Shader pointer
	 */
	template <class T, class... Args>
		requires std::derived_from<T, shader_base>
	uint CreateShader(Args&&... args);

	/** Register a shader by give constructed shader class, used by class Material */
	uint RegisterShader(luisa::unique_ptr<shader_base>&& Shader);

	/**
	 * Get shader by shader id, will return nullptr if the shader id is invalid
	 * @param ID Shader id
	 * @return Shader pointer
	 */
	[[nodiscard]] FORCEINLINE const shader_base* GetShader(uint ID) const;

	/**
	 * Get the default shader, i.e. disney shader
	 * @return Default shader id
	 */
	[[nodiscard]] constexpr uint GetDefaultShaderId() const { return 0; }
	[[nodiscard]] constexpr uint GetDisneyShaderId() const { return 0; };
	[[nodiscard]] constexpr uint GetBlinnPhongShaderId() const { return 1; }

	MaterialSceneProxy(const MaterialSceneProxy&) = delete;
	MaterialSceneProxy& operator=(const MaterialSceneProxy&) = delete;
	MaterialSceneProxy(MaterialSceneProxy&&) = delete;

	Polymorphic<shader_base> shader_call;


public:
	[[nodiscard]] Var<material_data> get_material_data(Expr<uint> material_index) const
	{
		return material_data_buffer->read(material_index);
	}


	/**
	 * Calculate material parameters for shading
	 * @param intersection the intersection point
	 * @return shader_id the shader id of the material, material_parameters calculated material parameters
	 */
	[[nodiscard]] std::pair<UInt, material_parameters>
	get_material_parameters(const ray_intersection& intersection) const noexcept;

	[[nodiscard]] Float3 brdf(const UInt& shader_id, const material_parameters& bxdf_parameters, const Float3& local_wo, const Float3& local_wi) const;

	[[nodiscard]] std::pair<Float3, Float> brdf_pdf(const UInt& shader_id, const material_parameters& bxdf_parameters, const Float3& local_wo, const Float3& local_wi) const;

	/**
	 * Sample the brdf
	 * @param shader_id the shader id
	 * @param bxdf_parameters the material parameters
	 * @param local_wo the local wo
	 * @param u the random number
	 * @return the sampled direction w_i(in local), brdf, pdf
	 */
	[[nodiscard]] std::tuple<Float3, Float3, Float> sample_brdf(const UInt& shader_id, const material_parameters& bxdf_parameters, const Float3& local_wo, const Float2& u) const;

protected:
	vector<material_data> MaterialDataVector;
	BufferView<material_data> material_data_buffer;

	THashMap<class Material*, uint>	MaterialIDMap;
	bool bNeedUpdate = false;
};

template <class T, class... Args>
	requires std::derived_from<T, shader_base>
uint MaterialSceneProxy::CreateShader(Args&&... args)
{
	return shader_call.create<T>(std::forward<Args>(args)...);
}

FORCEINLINE bool MaterialSceneProxy::IsMaterialUploaded(Material* InMaterial) const
{
	return MaterialIDMap.contains(InMaterial);
}

FORCEINLINE uint MaterialSceneProxy::GetMaterialID(Material* InMaterial)
{
	ASSERTMSG(MaterialIDMap.contains(InMaterial), "Material is not in the map, add to scene first!");
	return MaterialIDMap[InMaterial];
}

FORCEINLINE const shader_base* MaterialSceneProxy::GetShader(uint ID) const
{
	return shader_call.size() > ID ? shader_call.impl(ID) : nullptr;
}

FORCEINLINE bool MaterialSceneProxy::ShaderValid(uint ShaderId) const
{
	return shader_call.size() > ShaderId;
}
}
