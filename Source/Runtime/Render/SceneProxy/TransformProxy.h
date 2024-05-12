//
// Created by MarvelLi on 2024/4/5.
//

#pragma once
#include "SceneProxy.h"
#include "luisa/luisa-compute.h"
#include "Misc/Platform.h"
#include "Render/Core/transform_data.h"

class TransformComponent;
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


		Var<transform_data> get_transform_data(Expr<uint> transform_id) const
		{
			return transform_buffer->read(transform_id);
		}

	protected:
		static constexpr auto transform_matrix_buffer_size = 65536u;
		uint Id = 0;
		vector<transform_data> TransformDatas;
		map<TransformComponent*, uint> TransformIndexMap;
		set<TransformComponent*> DirtyTransforms;
		map<TransformSceneProxy*, uint> TransformInstanceMap;
		BufferView<transform_data> transform_buffer;
	};

	uint TransformSceneProxy::GetTransformCount() const noexcept
	{
		return Id;
	}
}