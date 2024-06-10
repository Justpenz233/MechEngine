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

	class ENGINE_API TransformSceneProxy : public SceneProxy
	{
	public:
		explicit TransformSceneProxy(RayTracingScene& InScene);

		virtual void UploadDirtyData(luisa::compute::Stream& stream) override;

		/**
		 * Try to add a new transform to the scene, if the transform already exists, return the existing id
		 * @param InTransform Transformcomponent to add
		 * @return The transform id
		 */
		uint AddTransform(TransformComponent* InTransform);

		/**
		 * Check if a transform exists in the scene
		 * @param InTransform Transformcomponent to check
		 * @return True if the transform exists
		 */
		bool IsExist(TransformComponent* InTransform) const;

		/**
		 * Check if a transform exists in the scene
		 * @param TransformID The transform id to check
		 * @return True if the transform exists
		 */
		bool IsExist(uint TransformID) const;

		/**
		* Try to update a transform render data, the transform may not exist in the scene
		* @param InTransform Transformcomponent to update
		*/
		void UpdateTransform(TransformComponent* InTransform);

		[[nodiscard]] FORCEINLINE uint GetTransformCount() const noexcept;

		[[nodiscard]] Var<transform_data> get_transform_data(Expr<uint> transform_id) const
		{
			return bindelss_buffer<transform_data>(bindless_id)->read(transform_id);
		}

	protected:
		static constexpr auto transform_matrix_buffer_size = 65536u;
		uint Id = 0;
		vector<transform_data> TransformDatas;
		map<TransformComponent*, uint> TransformIndexMap;
		set<TransformComponent*> DirtyTransforms;
		map<TransformSceneProxy*, uint> TransformInstanceMap;
		uint bindless_id;
		BufferView<transform_data> transform_buffer;
	};

	uint TransformSceneProxy::GetTransformCount() const noexcept
	{
		return Id;
	}
}