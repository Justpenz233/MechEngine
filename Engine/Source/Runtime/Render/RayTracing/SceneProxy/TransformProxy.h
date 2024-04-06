//
// Created by MarvelLi on 2024/4/5.
//

#pragma once
#include "SceneProxy.h"
#include "luisa/luisa-compute.h"
#include "Misc/Platform.h"
class TransformComponent;
namespace MechEngine::Rendering
{
struct transformData
{
	luisa::float4x4 transformMatrix{};
	luisa::float4 rotationQuaternion{};
	luisa::float3 scale{};
};
}

LUISA_STRUCT(MechEngine::Rendering::transformData, transformMatrix, rotationQuaternion, scale)
{
	[[nodiscard]] luisa::compute::Expr<luisa::compute::float3> get_location() const noexcept
	{
		return transformMatrix[3].xyz();
	}

	[[nodiscard]] luisa::compute::Expr<luisa::compute::float3> get_scale() const noexcept
	{
		return scale;
	}

	[[nodiscard]] luisa::compute::Expr<luisa::compute::float4x4> get_matrix() const noexcept
	{
		return transformMatrix;
	}

};

namespace MechEngine::Rendering
{
	using namespace luisa;
	using namespace luisa::compute;

	class TransformSceneProxy : public SceneProxy
	{
	public:
		explicit TransformSceneProxy(RayTracingScene& InScene);

		virtual void UploadDirtyData(luisa::compute::Stream& stream) override;

		uint AddTransform(TransformComponent* InTransform);

		void UpdateTransform(TransformComponent* InTransform);

		[[nodiscard]] FORCEINLINE uint GetTransformCount() const noexcept;


		Var<transformData> get_transform_data(Expr<uint> transform_id) const
		{
			return transform_buffer->read(transform_id);
		}

	protected:
		static constexpr auto transform_matrix_buffer_size = 65536u;
		uint Id = 0;
		vector<transformData> TransformDatas;
		map<TransformComponent*, uint> TransformIndexMap;
		set<TransformComponent*> DirtyTransforms;
		map<TransformSceneProxy*, uint> TransformInstanceMap;
		BufferView<transformData> transform_buffer;
	};

	uint TransformSceneProxy::GetTransformCount() const noexcept
	{
		return Id;
	}
}